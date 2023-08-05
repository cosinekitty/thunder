#pragma once

#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace Sapphire
{
    const double SPEED_OF_SOUND_IN_AIR = 343.0;     // [meters/second]

    struct BoltPoint
    {
        double x;
        double y;
        double z;

        BoltPoint()
            : x(0.0)
            , y(0.0)
            , z(0.0)
            {}

        BoltPoint(double _x, double _y, double _z)
            : x(_x)
            , y(_y)
            , z(_z)
            {}
    };


    using BoltPointList = std::vector<BoltPoint>;


    inline double Distance(const BoltPoint& a, const BoltPoint& b)
    {
        double dx = b.x - a.x;
        double dy = b.y - a.y;
        double dz = b.z - a.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }


    struct BoltSegment
    {
        BoltPoint a;
        BoltPoint b;

        BoltSegment()
            {}

        BoltSegment(const BoltPoint& _a, const BoltPoint& _b)
            : a(_a)
            , b(_b)
            {}
    };


    using BoltSegmentList = std::vector<BoltSegment>;


    class LightningBolt
    {
    private:
        BoltSegmentList seglist;
        const std::size_t maxSegments;
        double jag{};
        std::default_random_engine generator;
        std::normal_distribution<double> distribution{0.0, 1.0};

        BoltPoint randomHorizontal(double z, double radiusStandardDev)
        {
            // Pick a random vector parallel to the x-y plane, with zero z-displacement.
            double r = radiusStandardDev / M_SQRT2;
            double x = r * distribution(generator);
            double y = r * distribution(generator);
            return BoltPoint{x, y, z};
        }

        void crinkle(BoltPoint first, BoltPoint second, std::size_t budget)
        {
            if (budget == 0)
                throw std::logic_error("Cannot complete lightning fractal!");

            if (budget == 1)
            {
                seglist.push_back(BoltSegment{first, second});
            }
            else
            {
                BoltPoint midpoint{(first.x + second.x)/2, (first.y + second.y)/2, (first.z + second.z)/2};
                double disp = jag * Distance(first, second);
                midpoint.x += disp * distribution(generator);
                midpoint.y += disp * distribution(generator);
                midpoint.z += disp * distribution(generator);

                // Split the budget as equally as possible between the two halves of the fractal.
                // When the budget is an odd number, flip a coin to see who gets the extra coin.
                std::size_t firstBudget = budget / 2;
                std::size_t secondBudget = firstBudget;
                if (budget & 1)
                {
                    if (generator() & 1)
                        ++firstBudget;
                    else
                        ++secondBudget;
                }

                if (firstBudget + secondBudget != budget)
                    throw std::logic_error("Budget calculation error!");

                crinkle(first, midpoint, firstBudget);
                crinkle(midpoint, second, secondBudget);
            }
        }

    public:
        LightningBolt(std::size_t _maxSegments, unsigned _randomSeed = 0)
            : maxSegments(_maxSegments)
            , generator(_randomSeed)
        {
            // We must do all memory allocation at construction time.
            // Because LightningBolt can be part of an audio rendering pipeline,
            // we can't afford to allocate or free any memory once we start rendering.
            // Otherwise we risk unpredictable delays, which could cause audio stuttering.
            // Therefore, pre-reserve all memory we will need.
            // We will never go beyond the user-specified number of segments.
            seglist.reserve(_maxSegments);
        }

        std::size_t getMaxSegments() const
        {
            return maxSegments;
        }

        const BoltSegmentList& segments() const
        {
            return seglist;
        }

        void generate(double heightMeters = 3000.0, double radiusMeters = 1000.0, double jaggedness = 1.0)
        {
            seglist.clear();

            if (maxSegments > 0)
            {
                // Start with a single line segment representing the entire length of the lightning bolt.
                // The parameters `heightMeters` and `radiusMeters` define a cylindrical frame of reference
                // within which we maintain a loose confinement based on standard deviations of a normal distribution.
                BoltPoint top = randomHorizontal(heightMeters, radiusMeters);
                BoltPoint bottom = randomHorizontal(0.0, radiusMeters);

                // Recursively split the line segment into many crinkly line segments.
                jag = 0.15 * jaggedness;     // experimentally derived factor to create pleasing results for jaggedness = 1.0
                crinkle(top, bottom, maxSegments);
            }
        }
    };


    struct ThunderSegment
    {
        double distance1{};
        double distance2{};
    };


    inline bool operator < (const ThunderSegment& a, const ThunderSegment& b)       // for sorting
    {
        return a.distance1 < b.distance1;
    }


    using ThunderSegmentList = std::vector<ThunderSegment>;


    class Thunder
    {
    private:
        BoltPointList ears;
        const std::size_t maxSegments;
        std::vector<ThunderSegmentList> seglistForEar;
        double minDistance{};
        double maxDistance{};

        void startEar(const LightningBolt& bolt, const BoltPoint& ear, ThunderSegmentList& seglist)
        {
            seglist.clear();
            for (const BoltSegment& bs : bolt.segments())
            {
                // Calculate the distance of each of the bolt segment's endpoints to this ear.
                ThunderSegment ts;
                ts.distance1 = Distance(ear, bs.a);
                ts.distance2 = Distance(ear, bs.b);

                // Make sure the first distance is equal or closer than the second.
                if (ts.distance1 > ts.distance2)
                {
                    double swap = ts.distance1;
                    ts.distance1 = ts.distance2;
                    ts.distance2 = swap;
                }

                if (minDistance < 0.0)
                {
                    minDistance = ts.distance1;
                    maxDistance = ts.distance2;
                }
                else
                {
                    minDistance = std::min(minDistance, ts.distance1);
                    maxDistance = std::max(maxDistance, ts.distance2);
                }

                seglist.push_back(ts);
            }

            // Sort the segment list in ascending order of closer distances.
            std::sort(seglist.begin(), seglist.end());
        }

    public:
        Thunder(const BoltPointList& _ears, std::size_t _maxSegments)
            : ears(_ears)       // make a copy of the vector
            , maxSegments(_maxSegments)
            , seglistForEar(_ears.size())
        {
            for (ThunderSegmentList& slist : seglistForEar)
                slist.reserve(_maxSegments);
        }

        std::size_t numEars() const
        {
            return ears.size();
        }

        std::size_t getMaxSegments() const
        {
            return maxSegments;
        }

        const ThunderSegmentList& segments(std::size_t earIndex) const
        {
            return seglistForEar.at(earIndex);
        }

        void start(const LightningBolt& bolt)
        {
            if (bolt.getMaxSegments() > maxSegments)
                throw std::range_error("LightningBolt has too many segments for this Thunder object.");

            minDistance = maxDistance = -1.0;   // special flag for first time init
            const std::size_t n = ears.size();
            for (std::size_t i = 0; i < n; ++i)
                startEar(bolt, ears.at(i), seglistForEar.at(i));
        }

        std::vector<float> renderAudio(int sampleRateHz) const
        {
            std::vector<float> buffer;

            if (minDistance < maxDistance)
            {
                // For now, skip initial silence by starting at minDistance.
                // Later we can return minDistance and allow the caller to delay the inital onset for realism.
                // Calculate the total buffer duration in frames.

                double durationSeconds = (maxDistance - minDistance) / SPEED_OF_SOUND_IN_AIR;
                int durationFrames = static_cast<int>(std::ceil(sampleRateHz * durationSeconds));
                int nchannels = numEars();
                int nsamples = nchannels * durationFrames;

                buffer.clear();
                buffer.resize(nsamples);

                // Iterate through every ThunderSegment and mix in its contribution to the impulse response.
                // Each must be applied to the correct ear/channel.

                for (int c = 0; c < nchannels; ++c)
                {
                    for (const ThunderSegment& s : seglistForEar.at(c))
                    {
                        // Do a linear interpolation using the inverse square law at the range of distances.
                        double amp1 = 1 / (s.distance1 * s.distance1);
                        double amp2 = 1 / (s.distance2 * s.distance2);

                        // Snap to nearest frame at endpoints, but round down at the end.
                        // That is because another segment will usually snap to the endpoint as its beginning.
                        double t1 = (s.distance1 - minDistance) / SPEED_OF_SOUND_IN_AIR;
                        double t2 = (s.distance2 - minDistance) / SPEED_OF_SOUND_IN_AIR;
                        int f1 = static_cast<int>(std::round(t1 * sampleRateHz));
                        int f2 = static_cast<int>(std::round(t2 * sampleRateHz));
                        for (int f = f1; f < f2; ++f)
                        {
                            double x = static_cast<double>(f-f1) / static_cast<double>((f2-1) - f1);
                            buffer.at(nchannels*f + c) += (1.0-x)*amp1 + x*amp2;
                        }
                    }
                }
            }

            return buffer;
        }
    };
}
