#include "solution.hpp"
#define SOLUTION
static std::size_t mapToBucket(std::size_t v) {
                              //   size of a bucket
#ifdef SOLUTION
  constexpr std::array<std::size_t, 7> buckets = {13, 29, 41, 53, 71, 83, 100};
  std::size_t res = 0;
  for (auto threshold : buckets) {
    if (v >= threshold) {
      ++res;
    }
  }
  return res;
#else
  if      (v < 13)  return 0; //   13
  else if (v < 29)  return 1; //   16
  else if (v < 41)  return 2; //   12
  else if (v < 53)  return 3; //   12
  else if (v < 71)  return 4; //   18
  else if (v < 83)  return 5; //   12
  else if (v < 100) return 6; //   17
  return DEFAULT_BUCKET;
#endif
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
