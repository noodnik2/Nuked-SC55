#include "wav.h"

#include <cassert>
#include <cstring>

void WAV_WriteBytes(std::ofstream& output, const char* bytes, size_t len)
{
    output.write(bytes, len);
}

void WAV_WriteCString(std::ofstream& output, const char* s)
{
    WAV_WriteBytes(output, s, strlen(s));
}

void WAV_WriteU16LE(std::ofstream& output, uint16_t value)
{
    // TODO: don't assume LE host
    output.write((const char*)&value, sizeof(uint16_t));
}

void WAV_WriteU32LE(std::ofstream& output, uint32_t value)
{
    // TODO: don't assume LE host
    output.write((const char*)&value, sizeof(uint32_t));
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
    m_output.write((const char*)&frame.left, sizeof(int16_t));
    m_output.write((const char*)&frame.right, sizeof(int16_t));
    ++m_frames_written;
}

void WAV_Handle::Write(const AudioFrame<float>& frame)
{
    m_output.write((const char*)&frame.left, sizeof(float));
    m_output.write((const char*)&frame.right, sizeof(float));
    ++m_frames_written;
}

void WAV_Handle::Finish(uint32_t sample_rate)
{
    // go back and fill in the header
    m_output.seekp(0);

    if (m_format == AudioFormat::S16)
    {
        const uint32_t data_size = m_frames_written * sizeof(AudioFrame<int16_t>);

        // RIFF header
        WAV_WriteCString(m_output, "RIFF");
        WAV_WriteU32LE(m_output, 36 + data_size);
        WAV_WriteCString(m_output, "WAVE");
        // fmt
        WAV_WriteCString(m_output, "fmt ");
        WAV_WriteU32LE(m_output, 16);
        WAV_WriteU16LE(m_output, 1);
        WAV_WriteU16LE(m_output, 2);
        WAV_WriteU32LE(m_output, sample_rate);
        WAV_WriteU32LE(m_output, sample_rate * sizeof(AudioFrame<int16_t>));
        WAV_WriteU16LE(m_output, sizeof(AudioFrame<int16_t>));
        WAV_WriteU16LE(m_output, 16);
        // data
        WAV_WriteCString(m_output, "data");
        WAV_WriteU32LE(m_output, m_frames_written * sizeof(AudioFrame<int16_t>));

        assert(m_output.tellp() == 44);
    }
    else
    {
        const uint32_t data_size = m_frames_written * sizeof(AudioFrame<float>);

        // RIFF header
        WAV_WriteCString(m_output, "RIFF");
        WAV_WriteU32LE(m_output, 50 + data_size);
        WAV_WriteCString(m_output, "WAVE");
        // fmt
        WAV_WriteCString(m_output, "fmt ");
        WAV_WriteU32LE(m_output, 18);
        WAV_WriteU16LE(m_output, 3);
        WAV_WriteU16LE(m_output, 2);
        WAV_WriteU32LE(m_output, sample_rate);
        WAV_WriteU32LE(m_output, sample_rate * sizeof(AudioFrame<float>));
        WAV_WriteU16LE(m_output, sizeof(AudioFrame<float>));
        WAV_WriteU16LE(m_output, 32);
        WAV_WriteU16LE(m_output, 0);
        // fact
        WAV_WriteCString(m_output, "fact");
        WAV_WriteU32LE(m_output, 4);
        WAV_WriteU32LE(m_output, m_frames_written);
        // data
        WAV_WriteCString(m_output, "data");
        WAV_WriteU32LE(m_output, m_frames_written * sizeof(AudioFrame<float>));

        assert(m_output.tellp() == 58);
    }

    Close();
}

