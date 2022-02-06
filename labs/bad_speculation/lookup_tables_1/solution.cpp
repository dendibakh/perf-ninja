#include "solution.hpp"

static size_t mapToBucket(size_t v) {    // diff
  if (v >= 0 && v < 13)        return 0; // 13
  else if (v >= 13 && v < 29)  return 1; // 16
  else if (v >= 29 && v < 41)  return 2; // 12
  else if (v >= 41 && v < 53)  return 3; // 12
  else if (v >= 53 && v < 71)  return 4; // 18
  else if (v >= 71 && v < 83)  return 5; // 12
  else if (v >= 83 && v < 100) return 6; // 17
  return -1;  // let it crash
}

const std::array<int, 100> kLookupTable = []() -> std::array<int, 100> {
  std::array<int, 100> table;
  for (int i = 0; i < 100; i++) {
    table[i] = mapToBucket(i);
  }
  return table;
}();

std::array<size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[kLookupTable[v]]++;
  }
  return retBuckets;
}
