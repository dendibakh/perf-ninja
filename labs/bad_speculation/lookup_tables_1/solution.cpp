#include "solution.hpp"

static constexpr auto buildLUT() {
  constexpr int hi = 150;
  std::array<std::size_t, hi + 1> LUT{};
  for(int i = 0; i <= hi; i++) {
    if      (i < 13)  LUT[i] = 0;
    else if (i < 29)  LUT[i] = 1;
    else if (i < 41)  LUT[i] = 2;
    else if (i < 53)  LUT[i] = 3;
    else if (i < 71)  LUT[i] = 4;
    else if (i < 83)  LUT[i] = 5;
    else if (i < 100) LUT[i] = 6;
    else              LUT[i] = DEFAULT_BUCKET;
  }
  return LUT;
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  static constexpr auto LUT = buildLUT();
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[LUT[v]]++;
  }
  return retBuckets;
}
