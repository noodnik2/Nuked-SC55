// This is a very minimal WAVE writer. It only exists to output something other
// than raw sample data.

#pragma once

#include <fstream>
#include <cstdint>
#include <filesystem>
#include "audio.h"

class WAV_Handle
{
public:
    WAV_Handle() = default;
    ~WAV_Handle() = default;
    // moveable
    WAV_Handle(WAV_Handle&&) = default;
    WAV_Handle& operator=(WAV_Handle&&) = default;
    // noncopyable
    WAV_Handle(const WAV_Handle&) = delete;
    WAV_Handle& operator=(const WAV_Handle&) = delete;

    void Open(const char* filename, AudioFormat format);
    void Open(const std::filesystem::path& filename, AudioFormat format);
    void Close();
    void Write(const AudioFrame<int16_t>& frame);
    void Write(const AudioFrame<float>& frame);
    void Finish(uint32_t sample_rate);

private:
    std::ofstream m_output;
    uint64_t      m_frames_written = 0;
    AudioFormat   m_format;
};

