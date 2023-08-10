#pragma once

#include <vector>
#include <stdexcept>
#include "audio_buffer.hpp"

namespace Sapphire
{
    inline AudioBuffer InitConvolutionBuffer(const AudioBuffer& f, const AudioBuffer& g)
    {
        int channels = std::max(f.channels(), g.channels());
        int frames = f.frames() + g.frames();
        std::vector<float> data;
        data.resize(channels * frames);
        return AudioBuffer(data, channels);
    }

    inline void ConvolveChannelPair(
        AudioBuffer& y,
        const AudioBuffer& f,
        int fc,
        const AudioBuffer& g,
        int gc)
    {
        
    }

    inline AudioBuffer Convolution(const AudioBuffer& f, const AudioBuffer& g)
    {
        const int fc = f.channels();
        const int gc = g.channels();

        // Allow `f` to have any positive integer number of channels.
        if (fc == gc)
        {
            // Case 1: `f` and `g` have the same number of channels.
            // Convolve corresponding channels in `f` and `g` to produce the result.
            AudioBuffer y = InitConvolutionBuffer(f, g);
            for (int c = 0; c < fc; ++c)
                ConvolveChannelPair(y, f, c, g, c);
            return y;
        }

        if (gc == 1)
        {
            // Case 2: `f` has more than one channel and `g` has exactly one channel.
            // Convolve the single channel of `g` with every channel of `f` to produce
            // a result that has the same number of channels as `f`.
            AudioBuffer y = InitConvolutionBuffer(f, g);
            for (int c = 0; c < fc; ++c)
                ConvolveChannelPair(y, f, c, g, 0);
            return y;
        }

        if (fc == 1)
        {
            // Use recursion to flip `f` and `g`, resulting in a commutation of Case 2.
            return Convolution(g, f);
        }

        throw std::range_error("The audio buffers have an incompatible number of channels for convolution.");
    }
}
