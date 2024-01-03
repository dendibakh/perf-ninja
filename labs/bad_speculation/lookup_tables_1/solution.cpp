#include "solution.hpp"

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[(v >= 13) + (v >= 29) + (v >= 41) + (v >= 53) + (v >= 71) + (v >= 83)]++;
  }
  return retBuckets;
}
