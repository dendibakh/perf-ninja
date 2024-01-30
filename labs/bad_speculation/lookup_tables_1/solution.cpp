#include "solution.hpp"

std::array<int, 100> Lookup = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
};

static std::size_t mapToBucket(std::size_t v) {    // diff
    if (v < 0 || v > 100)
        return -1;  // let it crash

    return Lookup[v];
}


std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
    std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
    for (auto v: values) {
        retBuckets[mapToBucket(v)]++;
    }
    return retBuckets;
}
