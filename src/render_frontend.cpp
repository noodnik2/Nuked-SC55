#include "emu.h"
#include "smf.h"
#include "wav.h"
#include "ringbuffer.h"
#include "command_line.h"
#include "audio.h"
#include "cast.h"
#include <string>
#include <cstdio>
#include <charconv>
#include <chrono>
#include <cinttypes>

using namespace std::chrono_literals;

struct R_Parameters
{
    std::string_view input_filename;
    std::string_view output_filename;
    bool help = false;
    size_t instances = 1;
    EMU_SystemReset reset = EMU_SystemReset::NONE;
    std::filesystem::path rom_directory;
    AudioFormat output_format = AudioFormat::S16;
};

enum class R_ParseError
{
    Success,
    NoInput,
    NoOutput,
    MultipleInputs,
    InstancesInvalid,
    InstancesOutOfRange,
    UnexpectedEnd,
    RomDirectoryNotFound,
    FormatInvalid,
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
        case R_ParseError::InstancesInvalid:
            return "Instances couldn't be parsed (should be 1-16)";
        case R_ParseError::InstancesOutOfRange:
            return "Instances out of range (should be 1-16)";
        case R_ParseError::UnexpectedEnd:
            return "Expected another argument";
        case R_ParseError::RomDirectoryNotFound:
            return "Rom directory doesn't exist";
        case R_ParseError::FormatInvalid:
            return "Output format invalid";
    }
    return "Unknown error";
}

R_ParseError R_ParseCommandLine(int argc, char* argv[], R_Parameters& result)
{
    CommandLineReader reader(argc, argv);

    while (reader.Next())
    {
        if (reader.Any("-o"))
        {
            if (!reader.Next())
            {
                return R_ParseError::UnexpectedEnd;
            }

            result.output_filename = reader.Arg();
        }
        else if (reader.Any("-h", "--help", "-?"))
        {
            result.help = true;
            return R_ParseError::Success;
        }
        else if (reader.Any("-n", "--instances"))
        {
            if (!reader.Next())
            {
                return R_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.instances))
            {
                return R_ParseError::InstancesInvalid;
            }

            if (result.instances < 1 || result.instances > 16)
            {
                return R_ParseError::InstancesOutOfRange;
            }
        }
        else if (reader.Any("-r", "--reset"))
        {
            if (!reader.Next())
            {
                return R_ParseError::UnexpectedEnd;
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
        else if (reader.Any("-d", "--rom-directory"))
        {
            if (!reader.Next())
            {
                return R_ParseError::UnexpectedEnd;
            }

            result.rom_directory = reader.Arg();
            if (!std::filesystem::exists(result.rom_directory))
            {
                return R_ParseError::RomDirectoryNotFound;
            }
        }
        else if (reader.Any("-f", "--format"))
        {
            if (!reader.Next())
            {
                return R_ParseError::UnexpectedEnd;
            }

            if (reader.Arg() == "s16")
            {
                result.output_format = AudioFormat::S16;
            }
            else if (reader.Arg() == "f32")
            {
                result.output_format = AudioFormat::F32;
            }
            else
            {
                return R_ParseError::FormatInvalid;
            }
        }
        else
        {
            if (result.input_filename.size())
            {
                return R_ParseError::MultipleInputs;
            }
            result.input_filename = reader.Arg();
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

struct R_TrackRenderState
{
    Emulator emu;
    std::vector<AudioFrame<int16_t>> buffer_s16;
    std::vector<AudioFrame<float>> buffer_f32;
    size_t us_simulated = 0;
    const SMF_Track* track = nullptr;
    std::thread thread;

    // these fields are accessed from main thread during render process
    std::atomic<size_t> events_processed = 0;
    std::atomic<bool> done;

    void MixInto(std::vector<AudioFrame<int16_t>>& output)
    {
        if (output.size() < buffer_s16.size())
        {
            output.resize(buffer_s16.size(), AudioFrame<int16_t>{});
        }
        horizontal_sat_add_i16((int16_t*)output.data(), (int16_t*)buffer_s16.data(), (int16_t*)(buffer_s16.data() + buffer_s16.size()));
    }

    void MixInto(std::vector<AudioFrame<float>>& output)
    {
        if (output.size() < buffer_f32.size())
        {
            output.resize(buffer_f32.size(), AudioFrame<float>{});
        }
        horizontal_add_f32((float*)output.data(), (float*)buffer_f32.data(), (float*)(buffer_f32.data() + buffer_f32.size()));
    }

};

void R_ReceiveSample_S16(void* userdata, int32_t left, int32_t right)
{
    R_TrackRenderState* state = (R_TrackRenderState*)userdata;

    AudioFrame<int16_t> frame;
    frame.left = (int16_t)clamp<int32_t>(left >> 15, INT16_MIN, INT16_MAX);
    frame.right = (int16_t)clamp<int32_t>(right >> 15, INT16_MIN, INT16_MAX);

    state->buffer_s16.push_back(frame);
}

void R_ReceiveSample_F32(void* userdata, int32_t left, int32_t right)
{
    constexpr float DIV_REC = 1.0f / 536870912.0f;

    R_TrackRenderState* state = (R_TrackRenderState*)userdata;

    AudioFrame<float> frame;
    frame.left = (float)left * DIV_REC;
    frame.right = (float)right * DIV_REC;

    state->buffer_f32.push_back(frame);
}

void R_RunReset(Emulator& emu, EMU_SystemReset reset)
{
    if (reset == EMU_SystemReset::NONE)
    {
        return;
    }

    emu.PostSystemReset(reset);

    for (size_t i = 0; i < 24'000'000; ++i)
    {
        MCU_Step(emu.GetMCU());
    }
}

void R_PostEvent(Emulator& emu, const SMF_Data& data, const SMF_Event& ev)
{
    emu.PostMIDI(ev.status);
    emu.PostMIDI(ev.GetData(data.bytes));
}

struct R_TrackList
{
    std::vector<SMF_Track> tracks;
};

// Splits a track into `n` tracks, each track can be processed by a single
// emulator instance.
R_TrackList R_SplitTrackModulo(const SMF_Track& merged_track, size_t n)
{
    R_TrackList result;
    result.tracks.resize(n);

    for (auto& event : merged_track.events)
    {
        // System events need to be processed by all emulators
        if (event.IsSystem())
        {
            for (auto& dest : result.tracks)
            {
                dest.events.emplace_back(event);
            }
        }
        else
        {
            auto& dest = result.tracks[event.GetChannel() % n];
            dest.events.emplace_back(event);
        }
    }

    for (auto& track : result.tracks)
    {
        SMF_SetDeltasFromTimestamps(track);
    }

    return result;
}

void R_RenderOne(const SMF_Data& data, R_TrackRenderState& state)
{
    uint64_t division = data.header.division;
    uint64_t us_per_qn = 500000;

    const SMF_Track& track = (const SMF_Track&)*state.track;

    for (const SMF_Event& event : track.events)
    {
        const uint64_t this_event_time_us = state.us_simulated + SMF_TicksToUS(event.delta_time, us_per_qn, division);

        // Simulate until this event fires. We step twice because each step is
        // 12 cycles, and there are 24_000_000 cycles in a second.
        // 24_000_000 / 1_000_000 = 24 cycles per microsecond.
        while (state.us_simulated < this_event_time_us)
        {
            MCU_Step(state.emu.GetMCU());
            MCU_Step(state.emu.GetMCU());
            ++state.us_simulated;
        }

        if (event.IsTempo(data.bytes))
        {
            us_per_qn = event.GetTempoUS(data.bytes);
        }

        // Fire the event.
        if (!event.IsMetaEvent())
        {
            R_PostEvent(state.emu, data, event);
        }

        ++state.events_processed;
    }

    state.done = true;
}

void R_CursorUpLines(int n)
{
    printf("\x1b[%dF", n);
}

bool R_RenderTrack(const SMF_Data& data, const R_Parameters& params)
{
    const size_t instances = params.instances;

    // First combine all of the events so it's easier to process
    const SMF_Track merged_track = SMF_MergeTracks(data);
    // Then create a track specifically for each emulator instance
    const R_TrackList split_tracks = R_SplitTrackModulo(merged_track, instances);

    Romset rs = EMU_DetectRomset(params.rom_directory);
    printf("Detected romset: %s\n", EMU_RomsetName(rs));

    R_TrackRenderState render_states[SMF_CHANNEL_COUNT];
    for (size_t i = 0; i < instances; ++i)
    {
        render_states[i].emu.Init(EMU_Options {
            .enable_lcd = false,
        });

        if (!render_states[i].emu.LoadRoms(rs, params.rom_directory))
        {
            return false;
        }

        render_states[i].emu.Reset();

        printf("Running system reset for #%02" PRIu64 "...\n", i);
        R_RunReset(render_states[i].emu, params.reset);

        switch (params.output_format)
        {
            case AudioFormat::S16:
                render_states[i].emu.SetSampleCallback(R_ReceiveSample_S16, &render_states[i]);
                break;
            case AudioFormat::F32:
                render_states[i].emu.SetSampleCallback(R_ReceiveSample_F32, &render_states[i]);
                break;
            default:
                printf("Invalid audio format\n");
                return false;
        }

        render_states[i].track = &split_tracks.tracks[i];

        render_states[i].thread = std::thread(R_RenderOne, std::cref(data), std::ref(render_states[i]));
    }

    // Now we wait.
    bool all_done = false;
    while (!all_done)
    {
        all_done = true;

        for (size_t i = 0; i < instances; ++i)
        {
            if (!render_states[i].done)
            {
                all_done = false;
            }

            const size_t processed    = render_states[i].events_processed;
            const size_t total        = render_states[i].track->events.size();
            const float  percent_done = 100.f * (float)processed / (float)total;

            printf("#%02" PRIu64 " %6.2f%% [%" PRIu64 " / %" PRIu64 "]\n", i, percent_done, processed, total);
        }

        if (!all_done)
        {
            R_CursorUpLines(RangeCast<int>(instances));
        }

        std::this_thread::sleep_for(1000ms);
    }

    for (size_t i = 0; i < instances; ++i)
    {
        render_states[i].thread.join();
    }

    printf("Mixing final track and writing to disk...\n");

    WAV_Handle render_output;
    render_output.Open(params.output_filename, params.output_format);

    switch (params.output_format)
    {
        case AudioFormat::S16:
            {
                std::vector<AudioFrame<int16_t>> rendered_track;
                for (size_t instance = 0; instance < instances; ++instance)
                {
                    render_states[instance].MixInto(rendered_track);
                }
                for (const auto& frame : rendered_track)
                {
                    render_output.Write(frame);
                }
                break;
            }
        case AudioFormat::F32:
            {
                std::vector<AudioFrame<float>> rendered_track;
                for (size_t instance = 0; instance < instances; ++instance)
                {
                    render_states[instance].MixInto(rendered_track);
                }
                for (const auto& frame : rendered_track)
                {
                    render_output.Write(frame);
                }
                break;
            }
        default:
            printf("Invalid audio format\n");
            return false;
    }

    render_output.Finish(MCU_GetOutputFrequency(render_states[0].emu.GetMCU()));

    printf("Done!\n");

    return true;
}

void R_Usage(const char* prog_name)
{
    printf("Usage: %s <input>\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help                     Print this message\n");
    printf("  -o <filename>                  Render to filename\n");
    printf("  -n, --instances <instances>    Number of emulators to use (increases effective polyphony, longer to render)\n");
    printf("  -r, --reset gs|gm              Send GS or GM reset before rendering.\n");
    printf("  -d, --rom-directory <dir>      Sets the directory to load roms from.\n");
    printf("  -f, --format s16|f32           Set output format.\n");
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

    if (!R_RenderTrack(data, params))
    {
        printf("Failed to render track\n");
        return 1;
    }

    return 0;
}
