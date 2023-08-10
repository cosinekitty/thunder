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

    public:
        AudioBuffer(const std::vector<float>& _data, int _nChannels)
            : data(_data)
            , nChannels(_nChannels)
        {
            if (nChannels < 1)
                throw std::range_error("Invalid number of channels for AudioBuffer.");

            if (data.size() % static_cast<std::size_t>(nChannels) != 0)
                throw std::range_error("Data length is not an integer multiple of the channel count.");
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

        int index(int channel, int frame) const
        {
            return frame*nChannels + channel;
        }

        float& at(int channel, int frame)
        {
            return data.at(index(channel, frame));
        }

        const float& at(int channel, frame) const
        {
            return data.at(index(channel, frame));
        }
    };
}
