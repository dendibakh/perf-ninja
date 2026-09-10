#include "solution.hpp"

static constexpr std::size_t MAP_SIZE = 101;

static constexpr std::array<std::size_t, MAP_SIZE> createBucketMap() {
  std::array<std::size_t, MAP_SIZE> bucketMap{};
  int i = 0;
  for (; i < 13; ++i) bucketMap[i] = 0;
  for (; i < 29; ++i) bucketMap[i] = 1;
  for (; i < 41; ++i) bucketMap[i] = 2;
  for (; i < 53; ++i) bucketMap[i] = 3;
  for (; i < 71; ++i) bucketMap[i] = 4;
  for (; i < 83; ++i) bucketMap[i] = 5;
  for (; i < MAP_SIZE - 1; ++i) bucketMap[i] = 6;
  bucketMap[MAP_SIZE - 1] = DEFAULT_BUCKET;
  return bucketMap;
}

static std::size_t mapToBucket(std::size_t v) {
  static constexpr std::array<std::size_t, MAP_SIZE> BUCKET_MAP = createBucketMap();
  return BUCKET_MAP[std::min(v, MAP_SIZE - 1)];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
