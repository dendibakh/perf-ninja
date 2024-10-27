#include "solution.hpp"

static std::size_t mapToBucket(std::size_t v) {
                              //   size of a bucket
  if      (v < 13)  return 0; //   13
  else if (v < 29)  return 1; //   16
  else if (v < 41)  return 2; //   12
  else if (v < 53)  return 3; //   12
  else if (v < 71)  return 4; //   18
  else if (v < 83)  return 5; //   12
  else if (v < 100) return 6; //   17
  return DEFAULT_BUCKET;
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  std::array<std::size_t, 151> bucket_map;
  
  for (int i = 0; i < 13; ++i) {
    bucket_map[i] = 0;
  }
  for (int i = 13; i < 29; ++i) {
    bucket_map[i] = 1;
  }
  for (int i = 29; i < 41; ++i) {
    bucket_map[i] = 2;
  }
  for (int i = 41; i < 53; ++i) {
    bucket_map[i] = 3;
  }
  for (int i = 53; i < 71; ++i) {
    bucket_map[i] = 4;
  }
  for (int i = 71; i < 83; ++i) {
    bucket_map[i] = 5;
  }
  for (int i = 83; i < 100; ++i) {
    bucket_map[i] = 6;
  }

  for (int i = 100; i <= 150; ++i) {
    bucket_map[i] = DEFAULT_BUCKET;
  }
  for (auto v : values) {
    retBuckets[bucket_map[v]]++;
  }
  return retBuckets;
}
