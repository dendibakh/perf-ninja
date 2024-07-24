#include "solution.hpp"

class Precalc {
public:
  constexpr Precalc() {
    for (int v = 0; v < 100; v++) {
      if (v >= 0 && v < 13) {
        lookup[v] = 0;
      } else if (v >= 13 && v < 29) {
        lookup[v] = 1;
      } else if (v >= 29 && v < 41) {
        lookup[v] = 2;
      } else if (v >= 41 && v < 53) {
        lookup[v] = 3;
      } else if (v >= 53 && v < 71) {
        lookup[v] = 4;
      } else if (v >= 71 && v < 83) {
        lookup[v] = 5;
      } else if (v >= 83 && v < 100) {
        lookup[v] = 6;
      }
    }
  }

  size_t lookup[100]{0};
};

constexpr Precalc pre;
static std::size_t mapToBucket(std::size_t v) {    // diff
  return v >= 100 ? -1 : pre.lookup[v];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
