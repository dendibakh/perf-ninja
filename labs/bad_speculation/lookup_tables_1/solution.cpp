#include "solution.hpp"

int bucket_map[151] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-12
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13-28
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 29-40
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,// 41-52
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 53-70
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // 71-82
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 83-99
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // 100-109
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7
};

static std::size_t mapToBucket(std::size_t v) {
  return bucket_map[v];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
