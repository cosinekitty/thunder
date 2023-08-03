#pragma once

#include <vector>

namespace Sapphire
{
    struct BoltPoint
    {
        float x;
        float y;
        float z;

        BoltPoint()
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
            {}

        BoltPoint(float _x, float _y, float _z)
            : x(_x)
            , y(_y)
            , z(_z)
            {}
    };

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

    public:
        LightningBolt(std::size_t maxSegments)
        {
            seglist.reserve(maxSegments);
        }

        void generate(float heightMeters = 3000.0f, float radiusMeters = 1000.0f)
        {
            seglist.clear();
            seglist.push_back(BoltSegment(BoltPoint(0.0, 0.0, heightMeters), BoltPoint(radiusMeters/4, radiusMeters/2, 0.0)));
        }

        const BoltSegmentList& segments() const
        {
            return seglist;
        }
    };
}
