#include "emu.h"
#include "smf.h"
#include "wav.h"
#include "ringbuffer.h"
#include <string>
#include <cstdio>
#include <charconv>

struct R_Parameters
{
    std::string_view input_filename;
    std::string_view output_filename;
    bool help = false;
    size_t instances = 1;
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
            if (i < argc)
            {
                arg = argv[i];
                result.output_filename = arg;
            }
            else
            {
                return R_ParseError::UnexpectedEnd;
            }
        }
        else if (arg == "-h" || arg == "--help" || arg == "-?")
        {
            result.help = true;
            return R_ParseError::Success;
        }
        else if (arg == "-n" || arg == "--instances")
        {
            ++i;
            if (i >= argc)
            {
                return R_ParseError::UnexpectedEnd;
            }
            arg = argv[i];
            std::from_chars_result n = std::from_chars(arg.begin(), arg.end(), result.instances);
            if (n.ec != std::errc{})
            {
                return R_ParseError::InstancesInvalid;
            }
            if (result.instances < 1 || result.instances > 16)
            {
                return R_ParseError::InstancesOutOfRange;
            }
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

struct R_TrackRenderState
{
    Ringbuffer buffer;
    size_t render_offset = 0;
    size_t us_simulated = 0;

    void Flush(std::vector<AudioFrame>& output)
    {
        size_t output_offset = render_offset;
        size_t output_size = buffer.ReadableFrameCount();
        size_t output_last = output_offset + output_size;
        if (output_last > output.size())
        {
            // TODO: don't resize every flush
            output.resize(output_last, AudioFrame{});
        }
        buffer.ReadMix(&output[output_offset], output_size);
        render_offset += output_size;
    }
};

void R_ReceiveSample(void* userdata, int* sample)
{
    R_TrackRenderState* state = (R_TrackRenderState*)userdata;

    sample[0] >>= 15;
    sample[1] >>= 15;

    AudioFrame frame;
    frame.left = (int16_t)clamp<int>(sample[0], INT16_MIN, INT16_MAX);
    frame.right = (int16_t)clamp<int>(sample[1], INT16_MIN, INT16_MAX);

    state->buffer.Write(frame);
}

bool R_RenderTrack(const SMF_Data& data, std::string_view output_filename, size_t instances)
{
    // TODO: allow this directory to be configured
    Romset rs = EMU_DetectRomset(std::filesystem::path{});

    emu_t emus[SMF_CHANNEL_COUNT];
    R_TrackRenderState render_states[SMF_CHANNEL_COUNT];
    for (size_t i = 0; i < instances; ++i)
    {
        EMU_Init(emus[i], EMU_Options {
            .want_lcd = false,
        });

        if (!EMU_LoadRoms(emus[i], rs, std::filesystem::path{}))
        {
            return false;
        }

        EMU_Reset(emus[i]);

        render_states[i].buffer = Ringbuffer(16 * 1024);

        EMU_SetSampleCallback(emus[i], R_ReceiveSample, &render_states[i]);
    }

    WAV_Handle render_output;
    render_output.Open(output_filename);

    SMF_Track track = SMF_MergeTracks(data);

    uint64_t division = data.header.division;
    uint64_t us_per_qn = 500000;

    std::vector<AudioFrame> render_track;

    for (size_t i = 0; i < track.events.size(); ++i)
    {
        for (size_t instance = 0; instance < instances; ++instance)
        {
            const uint64_t this_event_time_us = render_states[instance].us_simulated + SMF_TicksToUS(track.events[i].delta_time, us_per_qn, division);

            printf("[%lld/%lld] #%02lld Event (%02x) at %lldus\r", i + 1, track.events.size(), instance, track.events[i].status, this_event_time_us);

            // Simulate until this event fires. We step twice because each step is
            // 12 cycles, and there are 24_000_000 cycles in a second.
            // 24_000_000 / 1_000_000 = 24 cycles per microsecond.
            while (render_states[instance].us_simulated < this_event_time_us)
            {
                render_states[instance].buffer.SetOversamplingEnabled(emus[instance].pcm->config_reg_3c & 0x40);

                MCU_Step(*emus[instance].mcu);
                MCU_Step(*emus[instance].mcu);
                ++render_states[instance].us_simulated;

                if (render_states[instance].buffer.IsFull())
                {
                    render_states[instance].Flush(render_track);
                }
            }
        }

        if (track.events[i].IsTempo(data.bytes))
        {
            us_per_qn = track.events[i].GetTempoUS(data.bytes);
        }

        // Map and fire the event.
        size_t destination = track.events[i].GetChannel() % instances;
        if (!track.events[i].IsMetaEvent())
        {
            MCU_PostUART(*emus[destination].mcu, track.events[i].status);
            for (uint32_t data_offset = track.events[i].data_first; data_offset != track.events[i].data_last; ++data_offset)
            {
                MCU_PostUART(*emus[destination].mcu, data.bytes[data_offset]);
            }
        }

    }

    printf("\n");

    for (size_t instance = 0; instance < instances; ++instance)
    {
        render_states[instance].Flush(render_track);
    }

    for (const auto& sample : render_track)
    {
        render_output.WriteSample(sample.left, sample.right);
    }

    render_output.Finish(MCU_GetOutputFrequency(*emus[0].mcu));

    return true;
}

void R_Usage(const char* prog_name)
{
    printf("usage: %s <input>\n", prog_name);
    printf("  -h, --help                     Print this message\n");
    printf("  -o <filename>                  Render to filename\n");
    printf("  -n, --instances <instances>    Number of emulators to use (increases effective polyphony, longer to render)\n");
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

    if (!R_RenderTrack(data, params.output_filename, params.instances))
    {
        printf("Failed to render track\n");
        return 1;
    }

    return 0;
}
