#include "solution.hpp"

constexpr size_t LUT_SIZE = 100;
static std::array<char, LUT_SIZE + 1> lut;

void prepareLUT() {
  for (int v = 0; v < LUT_SIZE; ++v) {
                                  //   size of a bucket
    if      (v < 13)  lut[v] = 0; //   13
    else if (v < 29)  lut[v] = 1; //   16
    else if (v < 41)  lut[v] = 2; //   12
    else if (v < 53)  lut[v] = 3; //   12
    else if (v < 71)  lut[v] = 4; //   18
    else if (v < 83)  lut[v] = 5; //   12
    else if (v < 100) lut[v] = 6; //   17
  }
  lut[LUT_SIZE] = DEFAULT_BUCKET;
}

static std::size_t mapToBucket(std::size_t v) {
  return lut[std::min(LUT_SIZE, v)];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
