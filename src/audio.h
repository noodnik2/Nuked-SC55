#pragma once

enum class AudioFormat
{
    S16,
    F32,
};

template <typename T>
struct AudioFrame {
    T left;
    T right;
};

