#include "solution.hpp"


constexpr char a[128] = {
  // 0..12
  0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 13..28
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  // 29..40
  2,2,2,2,2,2,2,2,2,2,2,2,
  // 41..52
  3,3,3,3,3,3,3,3,3,3,3,3,
  // 53..70
  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  // 71..82
  5,5,5,5,5,5,5,5,5,5,5,5,
  // 83..99
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  // 100..127 — always 7
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
 };



static std::size_t mapToBucket(std::size_t v) {
  if (v>=128) {
    return DEFAULT_BUCKET;
  }
  return a[v];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
