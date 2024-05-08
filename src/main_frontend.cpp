/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "emu.h"
#include "midi.h"
#include "ringbuffer.h"
#include "path_util.h"
#include "command_line.h"
#include <SDL.h>
#include <cinttypes>
#include <optional>

struct fe_emu_instance_t {
    Emulator     emu;
    Ringbuffer   sample_buffer;
    std::thread  thread;
    bool         running;
};

const size_t FE_MAX_INSTANCES = 16;

struct frontend_t {
    fe_emu_instance_t instances[FE_MAX_INSTANCES];
    size_t instances_in_use = 0;

    int audio_buffer_size = 0;
    int audio_page_size = 0;

    SDL_AudioDeviceID sdl_audio = 0;
};

struct FE_Parameters
{
    bool help = false;
    int port = 0;
    int audio_device_index = -1;
    int page_size = 512;
    int page_num = 32;
    bool autodetect = true;
    EMU_SystemReset reset = EMU_SystemReset::NONE;
    size_t instances = 1;
    Romset romset = Romset::MK2;
    std::optional<std::filesystem::path> rom_directory;
};

bool FE_AllocateInstance(frontend_t& container, fe_emu_instance_t** result)
{
    if (container.instances_in_use == FE_MAX_INSTANCES)
    {
        return false;
    }

    fe_emu_instance_t& fe = container.instances[container.instances_in_use];
    fe = fe_emu_instance_t();
    ++container.instances_in_use;

    if (result)
    {
        *result = &fe;
    }

    return true;
}

void FE_SendMIDI(frontend_t& fe, size_t n, uint8_t* first, uint8_t* last)
{
    fe.instances[n].emu.PostMIDI(std::span(first, last));
}

void FE_BroadcastMIDI(frontend_t& fe, uint8_t* first, uint8_t* last)
{
    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        FE_SendMIDI(fe, i, first, last);
    }
}

void FE_RouteMIDI(frontend_t& fe, uint8_t* first, uint8_t* last)
{
    if (*first < 0x80)
    {
        printf("FE_RouteMIDI received data byte %02x\n", *first);
        return;
    }

    const bool is_sysex = *first == 0xF0;
    const uint8_t channel = *first & 0x0F;

    if (is_sysex)
    {
        FE_BroadcastMIDI(fe, first, last);
    }
    else
    {
        FE_SendMIDI(fe, channel % fe.instances_in_use, first, last);
    }
}

void FE_ReceiveSample(void* userdata, int32_t left, int32_t right)
{
    fe_emu_instance_t& fe = *(fe_emu_instance_t*)userdata;

    AudioFrame frame;
    frame.left = (int16_t)clamp<int32_t>(left >> 15, INT16_MIN, INT16_MAX);
    frame.right = (int16_t)clamp<int32_t>(right >> 15, INT16_MIN, INT16_MAX);

    fe.sample_buffer.Write(frame);
}

void FE_AudioCallback(void* userdata, Uint8* stream, int len)
{
    frontend_t& frontend = *(frontend_t*)userdata;

    const size_t num_frames = len / sizeof(AudioFrame);
    memset(stream, 0, len);

    size_t renderable_count = num_frames;
    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        renderable_count = min(
            renderable_count,
            frontend.instances[i].sample_buffer.ReadableFrameCount()
        );
    }

    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        frontend.instances[i].sample_buffer.ReadMix((AudioFrame*)stream, renderable_count);
    }
}

static const char* audio_format_to_str(int format)
{
    switch(format)
    {
    case AUDIO_S8:
        return "S8";
    case AUDIO_U8:
        return "U8";
    case AUDIO_S16MSB:
        return "S16MSB";
    case AUDIO_S16LSB:
        return "S16LSB";
    case AUDIO_U16MSB:
        return "U16MSB";
    case AUDIO_U16LSB:
        return "U16LSB";
    case AUDIO_S32MSB:
        return "S32MSB";
    case AUDIO_S32LSB:
        return "S32LSB";
    case AUDIO_F32MSB:
        return "F32MSB";
    case AUDIO_F32LSB:
        return "F32LSB";
    }
    return "UNK";
}

bool FE_OpenAudio(frontend_t& fe, int deviceIndex, int pageSize, int pageNum)
{
    SDL_AudioSpec spec = {};
    SDL_AudioSpec spec_actual = {};

    fe.audio_page_size = (pageSize/2)*2; // must be even
    fe.audio_buffer_size = fe.audio_page_size*pageNum;

    // TODO: we just assume the first instance has the correct mcu type for
    // all instances, which is PROBABLY correct but maybe we want to do some
    // crazy stuff like running different mcu types concurrently in the future?
    const mcu_t& mcu = fe.instances[0].emu.GetMCU();
    
    spec.format = AUDIO_S16SYS;
    spec.freq = MCU_GetOutputFrequency(mcu);
    spec.channels = 2;
    spec.callback = FE_AudioCallback;
    spec.userdata = &fe;
    spec.samples = fe.audio_page_size / 4;
    
    int num = SDL_GetNumAudioDevices(0);
    if (num == 0)
    {
        printf("No audio output device found.\n");
        return false;
    }
    
    if (deviceIndex < -1 || deviceIndex >= num)
    {
        printf("Out of range audio device index is requested. Default audio output device is selected.\n");
        deviceIndex = -1;
    }
    
    const char* audioDevicename = deviceIndex == -1 ? "Default device" : SDL_GetAudioDeviceName(deviceIndex, 0);
    
    fe.sdl_audio = SDL_OpenAudioDevice(deviceIndex == -1 ? NULL : audioDevicename, 0, &spec, &spec_actual, 0);
    if (!fe.sdl_audio)
    {
        return false;
    }

    printf("Audio device: %s\n", audioDevicename);

    printf("Audio Requested: F=%s, C=%d, R=%d, B=%d\n",
           audio_format_to_str(spec.format),
           spec.channels,
           spec.freq,
           spec.samples);

    printf("Audio Actual: F=%s, C=%d, R=%d, B=%d\n",
           audio_format_to_str(spec_actual.format),
           spec_actual.channels,
           spec_actual.freq,
           spec_actual.samples);
    fflush(stdout);

    SDL_PauseAudioDevice(fe.sdl_audio, 0);

    return true;
}

void FE_RunInstance(fe_emu_instance_t& instance)
{
    MCU_WorkThread_Lock(instance.emu.GetMCU());
    while (instance.running)
    {
        instance.sample_buffer.SetOversamplingEnabled(instance.emu.GetPCM().config_reg_3c & 0x40);
        if (instance.sample_buffer.IsFull())
        {
            MCU_WorkThread_Unlock(instance.emu.GetMCU());
            while (instance.sample_buffer.IsFull())
            {
                SDL_Delay(1);
            }
            MCU_WorkThread_Lock(instance.emu.GetMCU());
        }

        MCU_Step(instance.emu.GetMCU());
    }
    MCU_WorkThread_Unlock(instance.emu.GetMCU());
}

void FE_Run(frontend_t& fe)
{
    bool working = true;

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        fe.instances[i].running = true;
        fe.instances[i].thread = std::thread(FE_RunInstance, std::ref(fe.instances[i]));
    }

    while (working)
    {
        for (size_t i = 0; i < fe.instances_in_use; ++i)
        {
            if (LCD_QuitRequested(fe.instances[i].emu.GetLCD()))
                working = false;

            LCD_Update(fe.instances[i].emu.GetLCD());
        }

        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event))
        {
            for (size_t i = 0; i < fe.instances_in_use; ++i)
            {
                LCD_HandleEvent(fe.instances[i].emu.GetLCD(), sdl_event);
            }
        }

        SDL_Delay(15);
    }

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        fe.instances[i].running = false;
        fe.instances[i].thread.join();
    }
}

bool FE_Init()
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize the SDL2: %s.\n", SDL_GetError());
        fflush(stderr);
        return false;
    }

    return true;
}

bool FE_CreateInstance(frontend_t& container, const std::filesystem::path& base_path, const FE_Parameters& params)
{
    fe_emu_instance_t* fe = nullptr;

    if (!FE_AllocateInstance(container, &fe))
    {
        fprintf(stderr, "ERROR: Failed to allocate instance.\n");
        return false;
    }

    if (!fe->emu.Init(EMU_Options { .enable_lcd = true }))
    {
        fprintf(stderr, "ERROR: Failed to init emulator.\n");
        return false;
    }

    fe->emu.SetSampleCallback(FE_ReceiveSample, fe);

    LCD_LoadBack(fe->emu.GetLCD(), base_path / "back.data");

    if (!fe->emu.LoadRoms(params.romset, *params.rom_directory))
    {
        fprintf(stderr, "ERROR: Failed to load roms.\n");
        return false;
    }
    fe->emu.Reset();

    if (!LCD_CreateWindow(fe->emu.GetLCD()))
    {
        fprintf(stderr, "ERROR: Failed to create window.\n");
        return false;
    }

    return true;
}

void FE_DestroyInstance(fe_emu_instance_t& fe)
{
    fe.running = false;
}

void FE_Quit(frontend_t& container)
{
    // Important to close audio devices first since this will stop the SDL
    // audio thread. Otherwise we might get a UAF destroying ringbuffers
    // while they're still in use.
    SDL_CloseAudioDevice(container.sdl_audio);
    for (size_t i = 0; i < container.instances_in_use; ++i)
    {
        FE_DestroyInstance(container.instances[i]);
    }
    MIDI_Quit();
    SDL_Quit();
}

enum class FE_ParseError
{
    Success,
    InstancesInvalid,
    InstancesOutOfRange,
    UnexpectedEnd,
    PageSizeInvalid,
    PageCountInvalid,
    UnknownArgument,
    PortInvalid,
    AudioDeviceInvalid,
    RomDirectoryNotFound,
};

const char* FE_ParseErrorStr(FE_ParseError err)
{
    switch (err)
    {
        case FE_ParseError::Success:
            return "Success";
        case FE_ParseError::InstancesInvalid:
            return "Instances couldn't be parsed (should be 1-16)";
        case FE_ParseError::InstancesOutOfRange:
            return "Instances out of range (should be 1-16)";
        case FE_ParseError::UnexpectedEnd:
            return "Expected another argument";
        case FE_ParseError::PageSizeInvalid:
            return "Page size invalid";
        case FE_ParseError::PageCountInvalid:
            return "Page count invalid";
        case FE_ParseError::UnknownArgument:
            return "Unknown argument";
        case FE_ParseError::PortInvalid:
            return "Port invalid";
        case FE_ParseError::AudioDeviceInvalid:
            return "Audio device invalid";
        case FE_ParseError::RomDirectoryNotFound:
            return "Rom directory doesn't exist";
    }
    return "Unknown error";
}

FE_ParseError FE_ParseCommandLine(int argc, char* argv[], FE_Parameters& result)
{
    CommandLineReader reader(argc, argv);

    while (reader.Next())
    {
        if (reader.Any("-h", "--help", "-?"))
        {
            result.help = true;
            return FE_ParseError::Success;
        }
        else if (reader.Any("-p", "--port"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.port))
            {
                return FE_ParseError::PortInvalid;
            }
        }
        else if (reader.Any("-a", "--audio-device"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.audio_device_index))
            {
                return FE_ParseError::AudioDeviceInvalid;
            }
        }
        else if (reader.Any("-b", "--buffer-size"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            std::string_view arg = reader.Arg();
            if (size_t colon = arg.find(':'); colon != std::string_view::npos)
            {
                auto page_size_sv = arg.substr(0, colon);
                auto page_num_sv  = arg.substr(colon + 1);

                if (!TryParse(page_size_sv, result.page_size))
                {
                    return FE_ParseError::PageSizeInvalid;
                }

                if (!TryParse(page_num_sv, result.page_num))
                {
                    return FE_ParseError::PageCountInvalid;
                }
            }
            else if (!reader.TryParse(result.page_size))
            {
                return FE_ParseError::PageSizeInvalid;
            }
        }
        else if (reader.Any("-r", "--reset"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (reader.Arg() == "gm")
            {
                result.reset = EMU_SystemReset::GM_RESET;
            }
            else if (reader.Arg() == "gs")
            {
                result.reset = EMU_SystemReset::GS_RESET;
            }
            else
            {
                result.reset = EMU_SystemReset::NONE;
            }
        }
        else if (reader.Any("-n", "--instances"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.instances))
            {
                return FE_ParseError::InstancesInvalid;
            }

            if (result.instances < 1 || result.instances > 16)
            {
                return FE_ParseError::InstancesOutOfRange;
            }
        }
        else if (reader.Any("-d", "--rom-directory"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.rom_directory = reader.Arg();

            if (!std::filesystem::exists(*result.rom_directory))
            {
                return FE_ParseError::RomDirectoryNotFound;
            }
        }
        else if (reader.Any("--mk2"))
        {
            result.romset = Romset::MK2;
            result.autodetect = false;
        }
        else if (reader.Any("--st"))
        {
            result.romset = Romset::ST;
            result.autodetect = false;
        }
        else if (reader.Any("--mk1"))
        {
            result.romset = Romset::MK1;
            result.autodetect = false;
        }
        else if (reader.Any("--cm300"))
        {
            result.romset = Romset::CM300;
            result.autodetect = false;
        }
        else if (reader.Any("--jv880"))
        {
            result.romset = Romset::JV880;
            result.autodetect = false;
        }
        else if (reader.Any("--scb55"))
        {
            result.romset = Romset::SCB55;
            result.autodetect = false;
        }
        else if (reader.Any("--rlp3237"))
        {
            result.romset = Romset::RLP3237;
            result.autodetect = false;
        }
        else if (reader.Any("-sc155"))
        {
            result.romset = Romset::SC155;
            result.autodetect = false;
        }
        else if (!reader.Any("-sc155mk2"))
        {
            result.romset = Romset::SC155MK2;
            result.autodetect = false;
        }
        else
        {
            return FE_ParseError::UnknownArgument;
        }
    }

    return FE_ParseError::Success;
}

void FE_Usage()
{
    std::string name = P_GetProcessPath().stem().generic_string();

    printf("Usage: %s [options]\n", name.c_str());
    printf("Options:\n");
    printf("  -h, --help, -?                                Display this information.\n");
    printf("\n");
    printf("  -p, --port          <port_number>             Set MIDI port.\n");
    printf("  -a, --audio-device  <device_number>           Set Audio Device index.\n");
    printf("  -b, --buffer-size   <page_size>:[page_count]  Set Audio Buffer size.\n");
    printf("  -n, --instances     <count>                   Set number of emulator instances.\n");
    printf("\n");
    printf("  -d, --rom-directory <dir>                     Set directory to look for ROMs in.\n");
    printf("  --mk2                                         Use SC-55mk2 ROM set.\n");
    printf("  --st                                          Use SC-55st ROM set.\n");
    printf("  --mk1                                         Use SC-55mk1 ROM set.\n");
    printf("  --cm300                                       Use CM-300/SCC-1 ROM set.\n");
    printf("  --jv880                                       Use JV-880 ROM set.\n");
    printf("  --scb55                                       Use SCB-55 ROM set.\n");
    printf("  --rlp3237                                     Use RLP-3237 ROM set.\n");
    printf("\n");
    printf("  -r, --reset        gs|gm                      Reset system in GS or GM mode.\n");
}

int main(int argc, char *argv[])
{
    FE_Parameters params;
    FE_ParseError result = FE_ParseCommandLine(argc, argv, params);
    if (result != FE_ParseError::Success)
    {
        printf("error: %s\n", FE_ParseErrorStr(result));
        return 1;
    }

    if (params.help)
    {
        FE_Usage();
        return 0;
    }

    frontend_t frontend;

    std::filesystem::path base_path = P_GetProcessPath().parent_path();

    if (std::filesystem::exists(base_path / "../share/nuked-sc55"))
        base_path = base_path / "../share/nuked-sc55";

    printf("Base path is: %s\n", base_path.generic_string().c_str());

    if (!params.rom_directory)
    {
        params.rom_directory = base_path;
    }

    printf("ROM directory is: %s\n", params.rom_directory->generic_string().c_str());

    if (params.autodetect)
    {
        params.romset = EMU_DetectRomset(*params.rom_directory);
        printf("ROM set autodetect: %s\n", EMU_RomsetName(params.romset));
    }

    if (!FE_Init())
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize frontend\n");
        return 1;
    }

    for (size_t i = 0; i < params.instances; ++i)
    {
        if (!FE_CreateInstance(frontend, base_path, params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to create instance %" PRIu64 "\n", i);
            return 1;
        }
    }

    if (!FE_OpenAudio(frontend, params.audio_device_index, params.page_size, params.page_num))
    {
        fprintf(stderr, "FATAL ERROR: Failed to open the audio stream.\n");
        fflush(stderr);
        return 1;
    }

    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        fe_emu_instance_t& fe = frontend.instances[i];
        fe.sample_buffer = Ringbuffer(frontend.audio_buffer_size / 2);
    }

    if (!MIDI_Init(frontend, params.port))
    {
        fprintf(stderr, "ERROR: Failed to initialize the MIDI Input.\nWARNING: Continuing without MIDI Input...\n");
        fflush(stderr);
    }

    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        frontend.instances[i].emu.PostSystemReset(params.reset);
    }

    FE_Run(frontend);

    FE_Quit(frontend);

    return 0;
}
