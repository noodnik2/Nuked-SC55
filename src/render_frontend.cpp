#include "emu.h"
#include "smf.h"
#include "wav.h"
#include "ringbuffer.h"
#include <string>
#include <cstdio>

struct R_Parameters
{
    std::string_view input_filename;
    std::string_view output_filename;
    bool help = false;
};

enum class R_ParseError
{
    Success,
    NoInput,
    NoOutput,
    MultipleInputs,
};

const char* R_ParseErrorStr(R_ParseError err)
{
    switch (err)
    {
        case R_ParseError::Success:
            return "Success";
        case R_ParseError::NoInput:
            return "No input file specified";
        case R_ParseError::NoOutput:
            return "No output file specified (pass -o)";
        case R_ParseError::MultipleInputs:
            return "Multiple input files";
    }
}

R_ParseError R_ParseCommandLine(int argc, char* argv[], R_Parameters& result)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];
        if (arg == "-o")
        {
            ++i;
            arg = argv[i];
            if (i < argc)
            {
                result.output_filename = arg;
            }
        }
        else if (arg == "-h" || arg == "--help" || arg == "-?")
        {
            result.help = true;
            return R_ParseError::Success;
        }
        else
        {
            if (result.input_filename.size())
            {
                return R_ParseError::MultipleInputs;
            }
            result.input_filename = arg;
        }
    }

    if (result.input_filename.size() == 0)
    {
        return R_ParseError::NoInput;
    }

    if (result.output_filename.size() == 0)
    {
        return R_ParseError::NoOutput;
    }

    return R_ParseError::Success;
}

void R_ReceiveSample(void* userdata, int* sample)
{
    WAV_Handle& wav = *(WAV_Handle*)userdata;
    sample[0] >>= 15;
    sample[1] >>= 15;

    AudioFrame frame;
    frame.left = (int16_t)clamp<int>(sample[0], INT16_MIN, INT16_MAX);
    frame.right = (int16_t)clamp<int>(sample[1], INT16_MIN, INT16_MAX);

    wav.WriteSample(frame.left, frame.right);
}

bool R_RenderTrack(const SMF_Data& data, std::string_view output_filename)
{
    emu_t emu;
    EMU_Init(emu, EMU_Options {
        .want_lcd = false,
    });

    Romset rs = EMU_DetectRomset(std::filesystem::path{});
    if (!EMU_LoadRoms(emu, rs, std::filesystem::path{}))
    {
        return false;
    }
    EMU_Reset(emu);

    WAV_Handle render_output;
    render_output.Open(output_filename);

    EMU_SetSampleCallback(emu, R_ReceiveSample, &render_output);

    SMF_Track track = SMF_MergeTracks(data);

    uint64_t division = data.header.division;
    uint64_t us_per_qn = 500000;
    uint64_t us_simulated = 0;

    for (size_t i = 0; i < track.events.size(); ++i) {
        uint64_t this_event_time_us = us_simulated + SMF_TicksToUS(track.events[i].delta_time, us_per_qn, division);

        if (track.events[i].IsTempo(data.bytes))
        {
            us_per_qn = track.events[i].GetTempoUS(data.bytes);
        }

        printf("[%lld/%lld] Event (%02x) at %lldus\r", i + 1, track.events.size(), track.events[i].status, this_event_time_us);

        // Simulate until this event fires. We step twice because each step is
        // 12 cycles, and there are 24_000_000 cycles in a second.
        // 24_000_000 / 1_000_000 = 24 cycles per microsecond.
        while (us_simulated < this_event_time_us)
        {
            MCU_Step(*emu.mcu);
            MCU_Step(*emu.mcu);
            ++us_simulated;
        }

        // Fire the event.
        if (!track.events[i].IsMetaEvent())
        {
            MCU_PostUART(*emu.mcu, track.events[i].status);
            for (uint32_t data_offset = track.events[i].data_first; data_offset != track.events[i].data_last; ++data_offset)
            {
                MCU_PostUART(*emu.mcu, data.bytes[data_offset]);
            }
        }
    }

    printf("\n");

    render_output.Finish(MCU_GetOutputFrequency(*emu.mcu));

    return true;
}

void R_Usage(const char* prog_name)
{
    printf("usage: %s <input>\n", prog_name);
    printf("  -h, --help        Print this message\n");
    printf("  -o <filename>     Render to filename\n");
    printf("\n");
}

int main(int argc, char* argv[])
{
    R_Parameters params;
    R_ParseError result = R_ParseCommandLine(argc, argv, params);

    if (result != R_ParseError::Success)
    {
        printf("error: %s\n", R_ParseErrorStr(result));
        R_Usage(argv[0]);
        return 1;
    }

    if (params.help)
    {
        R_Usage(argv[0]);
        return 0;
    }

    SMF_Data data;
    data = SMF_LoadEvents(params.input_filename);

    if (!R_RenderTrack(data, params.output_filename))
    {
        printf("Failed to render track\n");
        return 1;
    }

    return 0;
}
