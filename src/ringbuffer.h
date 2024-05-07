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
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "math_util.h"

struct AudioFrame {
    int16_t left;
    int16_t right;
};

class Ringbuffer {
public:
    Ringbuffer() = default;

    Ringbuffer(size_t frame_count)
        : m_frames(frame_count)
    {
    }

    void SetOversamplingEnabled(bool enabled)
    {
        m_oversampling = enabled;
        if (m_oversampling)
        {
            m_write_head &= ~1;
        }
    }

    bool IsFull() const
    {
        if (m_oversampling)
        {
            return ((m_write_head + 2) % m_frames.size()) == m_read_head;
        }
        else
        {
            return ((m_write_head + 1) % m_frames.size()) == m_read_head;
        }
    }

    void Write(const AudioFrame& frame)
    {
        m_frames[m_write_head] = frame;
        m_write_head = (m_write_head + 1) % m_frames.size();
    }

    size_t ReadableFrameCount() const
    {
        if (m_read_head <= m_write_head)
        {
            return m_write_head - m_read_head;
        }
        else
        {
            return m_frames.size() - (m_read_head - m_write_head);
        }
    }

    // Reads up to `frame_count` frames and returns the number of frames
    // actually read.
    size_t Read(AudioFrame* dest, size_t frame_count)
    {
        const size_t have_count = ReadableFrameCount();
        const size_t read_count = min(have_count, frame_count);
        size_t working_read_head = m_read_head;
        // TODO make this one or two memcpys
        for (size_t i = 0; i < read_count; ++i)
        {
            *dest = m_frames[working_read_head];
            ++dest;
            working_read_head = (working_read_head + 1) % m_frames.size();
        }
        m_read_head = working_read_head;
        return read_count;
    }

    // Reads up to `frame_count` frames and returns the number of frames
    // actually read. Mixes samples into dest by adding and clipping.
    size_t ReadMix(AudioFrame* dest, size_t frame_count)
    {
        const size_t have_count = ReadableFrameCount();
        const size_t read_count = min(have_count, frame_count);
        size_t working_read_head = m_read_head;
        for (size_t i = 0; i < read_count; ++i)
        {
            dest[i].left = saturating_add(dest[i].left, m_frames[working_read_head].left);
            dest[i].right = saturating_add(dest[i].right, m_frames[working_read_head].right);
            working_read_head = (working_read_head + 1) % m_frames.size();
        }
        m_read_head = working_read_head;
        return read_count;
    }

private:
    std::vector<AudioFrame> m_frames;
    size_t m_read_head = 0;
    size_t m_write_head = 0;
    bool m_oversampling = false;
};
