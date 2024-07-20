#include "solution.hpp"
#include <cstdint>

static std::size_t oldMapToBucket(std::size_t v) { // diff
    if (v >= 0 && v < 13)
        return 0; // 13
    else if (v >= 13 && v < 29)
        return 1; // 16
    else if (v >= 29 && v < 41)
        return 2; // 12
    else if (v >= 41 && v < 53)
        return 3; // 12
    else if (v >= 53 && v < 71)
        return 4; // 18
    else if (v >= 71 && v < 83)
        return 5; // 12
    else if (v >= 83 && v < 100)
        return 6; // 17
    return -1;    // let it crash
}

static std::size_t mapToBucket(std::size_t v) { // diff
    const static auto vals = [] {
        std::array<std::uint8_t, 100> ret;
        for (int v = 0; v < 100; ++v) {
            ret[v] = oldMapToBucket(v);
        }
        return ret;
    }();
    return v < 100 ? vals[v] : -1; // let it crash
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
    std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
    for (auto v: values) {
        retBuckets[mapToBucket(v)]++;
    }
    return retBuckets;
}
