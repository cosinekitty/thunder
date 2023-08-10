#pragma once

#include <vector>
#include <stdexcept>

namespace Sapphire
{
    class AudioBuffer
    {
    private:
        std::vector<float> data;
        int nChannels;

        static std::size_t DataLength(int frames, int channels)
        {
            if (frames < 0)
                throw std::range_error("Frame count is not allowed to be negative.");

            if (channels < 1)
                throw std::range_error("Channel count must be a positive integer.");

            return static_cast<std::size_t>(frames * channels);
        }

    public:
        // Construct an AudioBuffer with a pre-calculated float array.
        AudioBuffer(const std::vector<float>& _data, int _channels)
            : data(_data)
            , nChannels(_channels)
        {
            if (nChannels < 1)
                throw std::range_error("Invalid number of channels for AudioBuffer.");

            if (data.size() % static_cast<std::size_t>(nChannels) != 0)
                throw std::range_error("Data length is not an integer multiple of the channel count.");
        }

        // Construct an AudioBuffer with all zero samples for a frame count and channel count.
        AudioBuffer(int _frames, int _channels)
            : data(DataLength(_frames, _channels))
            , nChannels(_channels)
        {
        }

        const std::vector<float>& buffer() const
        {
            return data;
        }

        int channels() const
        {
            return nChannels;
        }

        int frames() const
        {
            return static_cast<int>(data.size()) / nChannels;
        }

        std::size_t index(int channel, int frame) const
        {
            return static_cast<std::size_t>(frame*nChannels + channel);
        }

        float& at(int channel, int frame)
        {
            return data.at(index(channel, frame));
        }

        float get(int channel, int frame) const
        {
            // Be more forgiving of going out of bounds.
            // Just return 0 for any data offset outside the valid range.
            const std::size_t i = index(channel, frame);
            return (i < data.size()) ? data[i] : 0.0f;
        }
    };
}
