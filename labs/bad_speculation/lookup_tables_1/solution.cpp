#include "solution.hpp"

static std::size_t mapToBucket(std::size_t v) {    // diff
  if (v >= 0 && v < 100)
    return (v >= 13) + (v >= 29) + (v >= 41) + (v >= 53) + (v >= 71) + (v >= 83);
  return -1;  // let it crash
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
