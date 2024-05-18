#include "wav.h"
#include "cast.h"

#include <bit>
#include <cassert>
#include <cstring>

// Constants from rfc2361
enum class WaveFormat : uint16_t
{
    PCM        = 0x0001,
    IEEE_FLOAT = 0x0003,
};

void WAV_WriteBytes(std::ofstream& output, const char* bytes, size_t len)
{
    output.write(bytes, RangeCast<std::streamsize>(len));
}

void WAV_WriteCString(std::ofstream& output, const char* s)
{
    WAV_WriteBytes(output, s, strlen(s));
}

void WAV_WriteU16LE(std::ofstream& output, uint16_t value)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        value = std::byteswap(value);
    }
    output.write((const char*)&value, sizeof(uint16_t));
}

void WAV_WriteU32LE(std::ofstream& output, uint32_t value)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        value = std::byteswap(value);
    }
    output.write((const char*)&value, sizeof(uint32_t));
}

void WAV_WriteF32LE(std::ofstream& output, float value)
{
    // byteswap is only implemented for integral types, so forward the call to
    // the U32 implementation
    WAV_WriteU32LE(output, std::bit_cast<uint32_t>(value));
}

void WAV_Handle::SetSampleRate(uint32_t sample_rate)
{
    m_sample_rate = sample_rate;
}

void WAV_Handle::Open(const char* filename, AudioFormat format)
{
    Open(std::filesystem::path(filename), format);
}

void WAV_Handle::Open(const std::filesystem::path& filename, AudioFormat format)
{
    m_format = format;
    m_output.open(filename, std::ios::binary);

    m_output.seekp(format == AudioFormat::S16 ? 44 : 58);
}

void WAV_Handle::Close()
{
    m_output.close();
}

void WAV_Handle::Write(const AudioFrame<int16_t>& frame)
{
    WAV_WriteU16LE(m_output, std::bit_cast<uint16_t>(frame.left));
    WAV_WriteU16LE(m_output, std::bit_cast<uint16_t>(frame.right));
    ++m_frames_written;
}

void WAV_Handle::Write(const AudioFrame<float>& frame)
{
    WAV_WriteF32LE(m_output, frame.left);
    WAV_WriteF32LE(m_output, frame.right);
    ++m_frames_written;
}

void WAV_Handle::Finish()
{
    // go back and fill in the header
    m_output.seekp(0);

    switch (m_format)
    {
    case AudioFormat::S16: {
        const uint32_t data_size = RangeCast<uint32_t>(m_frames_written * sizeof(AudioFrame<int16_t>));

        // RIFF header
        WAV_WriteCString(m_output, "RIFF");
        WAV_WriteU32LE(m_output, 36 + data_size);
        WAV_WriteCString(m_output, "WAVE");
        // fmt
        WAV_WriteCString(m_output, "fmt ");
        WAV_WriteU32LE(m_output, 16);
        WAV_WriteU16LE(m_output, (uint16_t)WaveFormat::PCM);
        WAV_WriteU16LE(m_output, AudioFrame<int16_t>::channel_count);
        WAV_WriteU32LE(m_output, m_sample_rate);
        WAV_WriteU32LE(m_output, m_sample_rate * sizeof(AudioFrame<int16_t>));
        WAV_WriteU16LE(m_output, sizeof(AudioFrame<int16_t>));
        WAV_WriteU16LE(m_output, 8 * sizeof(int16_t));
        // data
        WAV_WriteCString(m_output, "data");
        WAV_WriteU32LE(m_output, data_size);

        assert(m_output.tellp() == 44);

        break;
    }
    case AudioFormat::F32: {
        const uint32_t data_size = RangeCast<uint32_t>(m_frames_written * sizeof(AudioFrame<float>));

        // RIFF header
        WAV_WriteCString(m_output, "RIFF");
        WAV_WriteU32LE(m_output, 50 + data_size);
        WAV_WriteCString(m_output, "WAVE");
        // fmt
        WAV_WriteCString(m_output, "fmt ");
        WAV_WriteU32LE(m_output, 18);
        WAV_WriteU16LE(m_output, (uint16_t)WaveFormat::IEEE_FLOAT);
        WAV_WriteU16LE(m_output, AudioFrame<float>::channel_count);
        WAV_WriteU32LE(m_output, m_sample_rate);
        WAV_WriteU32LE(m_output, m_sample_rate * sizeof(AudioFrame<float>));
        WAV_WriteU16LE(m_output, sizeof(AudioFrame<float>));
        WAV_WriteU16LE(m_output, 8 * sizeof(float));
        WAV_WriteU16LE(m_output, 0);
        // fact
        WAV_WriteCString(m_output, "fact");
        WAV_WriteU32LE(m_output, 4);
        WAV_WriteU32LE(m_output, RangeCast<uint32_t>(m_frames_written));
        // data
        WAV_WriteCString(m_output, "data");
        WAV_WriteU32LE(m_output, data_size);

        assert(m_output.tellp() == 58);

        break;
    }
    }

    Close();
}
