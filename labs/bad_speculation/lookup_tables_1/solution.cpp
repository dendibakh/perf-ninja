#include "solution.hpp"

#include <cstdint>

alignas(64) static constexpr uint8_t buckets[100] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                 // 13
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,        // 16
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                    // 12
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,                    // 12
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  // 18
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,                    // 12
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,     // 16
};
inline static constexpr size_t mapToBucket(size_t v) {
  if (v < (sizeof(buckets) / sizeof(uint8_t))) return buckets[v];
  return DEFAULT_BUCKET;  // let it crash
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  alignas(64) std::array<std::size_t, NUM_BUCKETS> retBuckets{0};

  for (std::size_t i = 0; i + 3 < values.size(); i += 4) {
    int v0 = values[i], v1 = values[i + 1], v2 = values[i + 2],
        v3 = values[i + 3];
    retBuckets[mapToBucket(v0)]++;
    retBuckets[mapToBucket(v1)]++;
    retBuckets[mapToBucket(v2)]++;
    retBuckets[mapToBucket(v3)]++;
  }

  return retBuckets;
}
