#include "solution.hpp"
#define SOL

#ifdef SOL
#include <array>

// constexpr int makeLUT(int i)
// {
//     return (i < 13) ? 0 : (i < 29) ? 1 : (i < 41) ? 2 : (i < 53) ? 3 : (i < 71) ? 4 : (i < 83) ? 5 : (i < 100) ? 6 :
//     -1;
// }

// constexpr std::array<int, 100> generateBuckets()
// {
//     std::array<int, 100> b = {};
//     for (int i = 0; i < 100; ++i)
//         b[i] = makeLUT(i);
//     return b;
// }

alignas(64) constexpr std::array<int, 100> buckets = {
    // generateBuckets();
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                // 13
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,       // 16
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                   // 12
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,                   // 12
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 18
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,                   // 12
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6     // 16
};
static size_t mapToBucket(size_t v)
{
    if (v < 100)
        return buckets[v];
    return DEFAULT_BUCKET;
}
#else
static std::size_t mapToBucket(std::size_t v)
{
    //   size of a bucket
    if (v < 13)
        return 0; //   13
    else if (v < 29)
        return 1; //   16
    else if (v < 41)
        return 2; //   12
    else if (v < 53)
        return 3; //   12
    else if (v < 71)
        return 4; //   18
    else if (v < 83)
        return 5; //   12
    else if (v < 100)
        return 6; //   17
    return DEFAULT_BUCKET;
}
#endif

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values)
{
    std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
    for (auto v : values)
    {
        retBuckets[mapToBucket(v)]++;
    }
    return retBuckets;
}
