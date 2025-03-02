#include "output_asio.h"

#include "asio_config.h"

#include "asiosys.h"
// asiosys *must* be included before these headers
#include "asio.h"
#include "asiodrivers.h"

#include "ringbuffer.h"
#include <atomic>
#include <cinttypes>

// number of buffers, one per stereo channel
const size_t N_BUFFERS = 2;

// one per instance
const size_t MAX_STREAMS = 16;

struct GlobalAsioState
{
    AsioDrivers    drivers;
    ASIODriverInfo driver_info;
    ASIOCallbacks  callbacks;

    ASIOBufferInfo   buffer_info[N_BUFFERS]{};
    ASIOChannelInfo  channel_info[N_BUFFERS]{};
    SDL_AudioStream* streams[MAX_STREAMS]{};
    size_t           stream_count = 0;

    // Size of a buffer as requested by ASIO driver
    long min_size;
    long max_size;
    long preferred_size;
    long granularity;

    // Size of a buffer as requested by the user
    long user_size;

    // Size of a buffer as it will be used
    size_t buffer_size_bytes;
    size_t buffer_size_frames;

    long input_channel_count;
    long output_channel_count;

    std::atomic<bool> defer_reset;

    ASIOSampleType output_type;

    // Contains interleaved frames received from individual `streams`.
    // This is necessarily 2 * `buffer_size_bytes` long.
    GenericBuffer mix_buffer;
};

// there isn't a way around using globals here, the ASIO API doesn't accept arbitrary userdata in its callbacks
static GlobalAsioState g_asio_state;

// defined in ASIO SDK
// we do actually need to do the loading through this function or else we'll segfault on exit
bool loadAsioDriver(char* name);

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess);
static void      bufferSwitch(long index, ASIOBool processNow);
static void      sampleRateDidChange(ASIOSampleRate sRate);
static long      asioMessage(long selector, long value, void* message, double* opt);

static const char* SampleTypeToString(ASIOSampleType type)
{
    switch (type)
    {
    case ASIOSTInt16MSB:
        return "ASIOSTInt16MSB";
    case ASIOSTInt24MSB:
        return "ASIOSTInt24MSB";
    case ASIOSTInt32MSB:
        return "ASIOSTInt32MSB";
    case ASIOSTFloat32MSB:
        return "ASIOSTFloat32MSB";
    case ASIOSTFloat64MSB:
        return "SIOSTFloat64MSB";
    case ASIOSTInt32MSB16:
        return "ASIOSTInt32MSB16";
    case ASIOSTInt32MSB18:
        return "ASIOSTInt32MSB18";
    case ASIOSTInt32MSB20:
        return "ASIOSTInt32MSB20";
    case ASIOSTInt32MSB24:
        return "ASIOSTInt32MSB24";
    case ASIOSTInt16LSB:
        return "ASIOSTInt16LSB";
    case ASIOSTInt24LSB:
        return "ASIOSTInt24LSB";
    case ASIOSTInt32LSB:
        return "ASIOSTInt32LSB";
    case ASIOSTFloat32LSB:
        return "ASIOSTFloat32LSB";
    case ASIOSTFloat64LSB:
        return "ASIOSTFloat64LSB";
    case ASIOSTInt32LSB16:
        return "ASIOSTInt32LSB16";
    case ASIOSTInt32LSB18:
        return "ASIOSTInt32LSB18";
    case ASIOSTInt32LSB20:
        return "ASIOSTInt32LSB20";
    case ASIOSTInt32LSB24:
        return "ASIOSTInt32LSB24";
    case ASIOSTDSDInt8LSB1:
        return "ASIOSTDSDInt8LSB1";
    case ASIOSTDSDInt8MSB1:
        return "ASIOSTDSDInt8MSB1";
    case ASIOSTDSDInt8NER8:
        return "ASIOSTDSDInt8NER8";
    default:
        return "Unknown sample type";
    }
}

bool Out_ASIO_QueryOutputs(AudioOutputList& list)
{
    const size_t MAX_NAMES    = 32;
    const size_t MAX_NAME_LEN = 32;

    // TODO: wat. do we seriously need to allocate all this?
    char* names[MAX_NAMES];
    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        names[i] = (char*)malloc(MAX_NAME_LEN);
    }

    long names_count = g_asio_state.drivers.getDriverNames(names, MAX_NAMES);
    for (long i = 0; i < names_count; ++i)
    {
        list.push_back({.name = names[i], .kind = AudioOutputKind::ASIO});
    }

    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        free(names[i]);
    }

    return true;
}

bool Out_ASIO_Create(const char* driver_name)
{
    // for some reason the api wants a non-const pointer
    char internal_driver_name[256]{};
    if (strcpy_s(internal_driver_name, sizeof(internal_driver_name), driver_name) != 0)
    {
        fprintf(stderr, "Driver name too long: `%s`\n", driver_name);
        return false;
    }

    if (!loadAsioDriver(internal_driver_name))
    {
        fprintf(stderr, "Failed to load ASIO driver `%s`\n", internal_driver_name);
        return false;
    }

    if (ASIOInit(&g_asio_state.driver_info) != ASE_OK)
    {
        fprintf(stderr, "ASIOInit failed\n");
        return false;
    }

    fprintf(stderr,
            "asioVersion:   %ld\n"
            "driverVersion: %ld\n"
            "name:          %s\n"
            "errorMessage:  %s\n",
            g_asio_state.driver_info.asioVersion,
            g_asio_state.driver_info.driverVersion,
            g_asio_state.driver_info.name,
            g_asio_state.driver_info.errorMessage);

    if (ASIOGetBufferSize(
            &g_asio_state.min_size, &g_asio_state.max_size, &g_asio_state.preferred_size, &g_asio_state.granularity) !=
        ASE_OK)
    {
        fprintf(stderr, "ASIOGetBufferSize failed\n");
        ASIOExit();
        return false;
    }

    fprintf(stderr,
            "ASIOGetBufferSize (min: %ld, max: %ld, preferred: %ld, granularity: %ld);\n",
            g_asio_state.min_size,
            g_asio_state.max_size,
            g_asio_state.preferred_size,
            g_asio_state.granularity);

    // ASIO4ALL can't handle the sample rate the emulator uses, so we'll need
    // to use a more common one and resample
    if (ASIOSetSampleRate(44100) != ASE_OK)
    {
        fprintf(stderr, "ASIOSetSampleRate(44100) failed; trying to continue anyways\n");
    }

    if (ASIOGetChannels(&g_asio_state.input_channel_count, &g_asio_state.output_channel_count))
    {
        fprintf(stderr, "ASIOGetChannels failed\n");
        ASIOExit();
        return false;
    }

    fprintf(stderr,
            "Available channels: %ld in, %ld out\n",
            g_asio_state.input_channel_count,
            g_asio_state.output_channel_count);

    if ((size_t)g_asio_state.output_channel_count < N_BUFFERS)
    {
        fprintf(stderr, "%" PRIu64 " channels required; aborting\n", N_BUFFERS);
        ASIOExit();
        return false;
    }

    for (size_t i = 0; i < N_BUFFERS; ++i)
    {
        g_asio_state.buffer_info[i].isInput    = ASIOFalse;
        g_asio_state.buffer_info[i].channelNum = (long)i;
        g_asio_state.buffer_info[i].buffers[0] = nullptr;
        g_asio_state.buffer_info[i].buffers[1] = nullptr;
    }

    g_asio_state.callbacks.bufferSwitch         = bufferSwitch;
    g_asio_state.callbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
    g_asio_state.callbacks.sampleRateDidChange  = sampleRateDidChange;
    g_asio_state.callbacks.asioMessage          = asioMessage;

    // Size in frames can be determined now, but we need to wait until after we get a format from the driver to
    // determine size in bytes
    g_asio_state.buffer_size_frames = (size_t)g_asio_state.user_size;

    if (ASIOCreateBuffers(
            g_asio_state.buffer_info, N_BUFFERS, (long)g_asio_state.buffer_size_frames, &g_asio_state.callbacks) !=
        ASE_OK)
    {
        fprintf(stderr, "ASIOCreateBuffers failed\n");
        ASIOExit();
        return false;
    }

    for (size_t i = 0; i < N_BUFFERS; ++i)
    {
        g_asio_state.channel_info[i].channel = g_asio_state.buffer_info[i].channelNum;
        ASIOGetChannelInfo(&g_asio_state.channel_info[i]);

        fprintf(stderr,
                "ASIO channel %" PRIu64 ": %s: %s\n",
                i,
                g_asio_state.channel_info[i].name,
                SampleTypeToString(g_asio_state.channel_info[i].type));
    }

    g_asio_state.output_type = g_asio_state.channel_info[0].type;

    for (size_t i = 1; i < N_BUFFERS; ++i)
    {
        if (g_asio_state.output_type != g_asio_state.channel_info[i].type)
        {
            fprintf(stderr, "ASIO channel %" PRIu64 " has a different output type!\n", i);
            ASIOExit();
            return false;
        }
    }

    // Output type acquired, now we know the actual size of the buffer
    g_asio_state.buffer_size_bytes = g_asio_state.buffer_size_frames * Out_ASIO_GetFormatSampleSizeBytes();

    // *2 because an ASIO buffer only represents one channel, but our mix buffer will hold 2 channels
    g_asio_state.mix_buffer.Free();
    g_asio_state.mix_buffer.Init(2 * g_asio_state.buffer_size_bytes);

    return true;
}

void Out_ASIO_Destroy()
{
    ASIOStop();
    ASIODisposeBuffers();
    ASIOExit();
}

bool Out_ASIO_Start()
{
    if (ASIOStart() != ASE_OK)
    {
        fprintf(stderr, "ASIOStart failed\n");
        return false;
    }

    return true;
}

void Out_ASIO_AddStream(SDL_AudioStream* stream)
{
    if (g_asio_state.stream_count == MAX_STREAMS)
    {
        fprintf(stderr, "PANIC: attempted to add more than %" PRIu64 " ASIO streams\n", MAX_STREAMS);
        exit(1);
    }
    g_asio_state.streams[g_asio_state.stream_count] = stream;
    ++g_asio_state.stream_count;
}

double Out_ASIO_GetFrequency()
{
    ASIOSampleRate rate;
    ASIOGetSampleRate(&rate);
    return rate;
}

SDL_AudioFormat Out_ASIO_GetFormat()
{
    switch (g_asio_state.output_type)
    {
    case ASIOSTInt16LSB:
        return AUDIO_S16LSB;
    case ASIOSTInt32LSB:
        return AUDIO_S32LSB;
    case ASIOSTFloat32LSB:
        return AUDIO_F32LSB;
    case ASIOSTInt16MSB:
        return AUDIO_S16MSB;
    case ASIOSTInt32MSB:
        return AUDIO_S32MSB;
    case ASIOSTFloat32MSB:
        return AUDIO_F32MSB;
    default:
        fprintf(stderr, "PANIC: ASIO format conversion not implemented\n");
        exit(1);
    }
}

size_t Out_ASIO_GetFormatSampleSizeBytes()
{
    return SDL_AUDIO_BITSIZE(Out_ASIO_GetFormat()) / 8;
}

size_t Out_ASIO_GetFormatFrameSizeBytes()
{
    return 2 * Out_ASIO_GetFormatSampleSizeBytes();
}

void Out_ASIO_Stop()
{
    ASIOStop();
}

bool Out_ASIO_IsResetRequested()
{
    return g_asio_state.defer_reset;
}

bool Out_ASIO_Reset()
{
    g_asio_state.defer_reset = false;
    Out_ASIO_Destroy();

    if (!Out_ASIO_Create(g_asio_state.driver_info.name))
    {
        fprintf(stderr, "ASIO reset: failed to re-initialize ASIO");
        return false;
    }

    if (!Out_ASIO_Start())
    {
        fprintf(stderr, "ASIO reset: failed to restart ASIO playback");
        return false;
    }

    return true;
}

void Out_ASIO_SetBufferSize(int size)
{
    g_asio_state.user_size = size;
}

int Out_ASIO_GetBufferSize()
{
    return g_asio_state.user_size;
}

// `src` contains `count` pairs of 16-bit words LRLRLRLR (here count = 4)
// `dst_a` will receive LLLL
// `dst_b` will receive RRRR
inline void Deinterleave16(void* dst_a, void* dst_b, const void* src, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        memcpy((uint8_t*)dst_a + 2 * i, (uint8_t*)src + 4 * i + 0, 2);
        memcpy((uint8_t*)dst_b + 2 * i, (uint8_t*)src + 4 * i + 2, 2);
    }
}

// same as above but for 32-bit words
inline void Deinterleave32(void* dst_a, void* dst_b, const void* src, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        memcpy((uint8_t*)dst_a + 4 * i, (uint8_t*)src + 8 * i + 0, 4);
        memcpy((uint8_t*)dst_b + 4 * i, (uint8_t*)src + 8 * i + 4, 4);
    }
}

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess)
{
    (void)params;
    (void)directProcess;

    size_t renderable_frames = (size_t)g_asio_state.buffer_size_frames;
    for (size_t i = 0; i < g_asio_state.stream_count; ++i)
    {
        renderable_frames = Min(
            renderable_frames, (size_t)SDL_AudioStreamAvailable(g_asio_state.streams[i]) / sizeof(AudioFrame<int32_t>));
    }

    memset(g_asio_state.buffer_info[0].buffers[index], 0, g_asio_state.buffer_size_bytes);
    memset(g_asio_state.buffer_info[1].buffers[index], 0, g_asio_state.buffer_size_bytes);

    if (renderable_frames >= (size_t)g_asio_state.buffer_size_frames)
    {
        for (size_t i = 0; i < g_asio_state.stream_count; ++i)
        {
            SDL_AudioStreamGet(g_asio_state.streams[i],
                               g_asio_state.mix_buffer.DataFirst(),
                               (int)g_asio_state.mix_buffer.GetByteLength());

            switch (Out_ASIO_GetFormatSampleSizeBytes())
            {
            case 4:
                Deinterleave32(g_asio_state.buffer_info[0].buffers[index],
                               g_asio_state.buffer_info[1].buffers[index],
                               g_asio_state.mix_buffer.DataFirst(),
                               g_asio_state.buffer_size_frames);
                break;
            case 2:
                Deinterleave16(g_asio_state.buffer_info[0].buffers[index],
                               g_asio_state.buffer_info[1].buffers[index],
                               g_asio_state.mix_buffer.DataFirst(),
                               g_asio_state.buffer_size_frames);
                break;
            default:
                fprintf(stderr, "PANIC: Deinterleave not implemented for this sample size\n");
                exit(1);
            }
        }
    }

    // TODO: supposed to query if this optimization is available
    ASIOOutputReady();

    return 0;
}

static void bufferSwitch(long index, ASIOBool processNow)
{
    ASIOTime timeInfo;
    memset(&timeInfo, 0, sizeof(timeInfo));
    bufferSwitchTimeInfo(&timeInfo, index, processNow);
}

static void sampleRateDidChange(ASIOSampleRate sRate)
{
    fprintf(stderr, "ASIO: sample rate changed to %f\n", sRate);
}

static long asioMessage(long selector, long value, void* message, double* opt)
{
    (void)value;
    (void)message;
    (void)opt;

    switch (selector)
    {
    case kAsioEngineVersion:
        return 2;
    case kAsioSupportsTimeCode:
        return 0;
    case kAsioResetRequest:
        g_asio_state.defer_reset = true;
        return 1;
    default:
        return 1;
    }
}
