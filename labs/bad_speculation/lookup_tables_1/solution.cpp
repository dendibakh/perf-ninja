#include "solution.hpp"

#include <cstdint>

alignas(64) static constexpr std::array<std::uint8_t, 101> buckets = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            // 13
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 16
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                               // 12
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,                               // 12
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,             // 18
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,                               // 12
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, DEFAULT_BUCKET // 16
};
inline static constexpr size_t mapToBucket(int v)
{
    // const size_t mask = -(v < 100); // 0xFFFFFFFF... if true, 0x0 if false
    // size_t clamped = (v & mask) | (100 & ~mask);

    return buckets[(v < 100) ? v : 100]; // comparison as int before implicit conversion
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values)
{
    alignas(64) std::array<std::size_t, NUM_BUCKETS> retBuckets{0};

    for (std::size_t i = 0; i + 3 < values.size(); i += 4)
    {
        int v0 = values[i], v1 = values[i + 1], v2 = values[i + 2], v3 = values[i + 3];
        retBuckets[mapToBucket(v0)]++;
        retBuckets[mapToBucket(v1)]++;
        retBuckets[mapToBucket(v2)]++;
        retBuckets[mapToBucket(v3)]++;
    }

    return retBuckets;
}
