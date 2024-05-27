#pragma once

#include <cstddef>

enum class AudioFormat
{
    S16,
    S32,
    F32,
};

template <typename T>
struct AudioFrame {
    T left;
    T right;

    static constexpr size_t channel_count = 2;
};

