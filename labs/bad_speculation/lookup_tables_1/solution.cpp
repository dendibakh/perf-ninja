#include "solution.hpp"
#include <cstdio>


#define SOLUTION_2
#ifdef SOLUTION_1
  constexpr uint8_t look_up[101] { [0 ... 12] = 0, [13 ... 28] = 1, [29 ... 40] = 2, [41 ... 52] = 3, [53 ... 70] = 4, [71 ... 82] = 5, [83 ... 99] = 6, [100] = DEFAULT_BUCKET};
#elif defined(SOLUTION_2)
uint8_t look_up[100] = {
// 1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};
#endif

static std::size_t mapToBucket(std::size_t v) {
                              //   size of a bucket

  #ifdef SOLUTION_1
    return look_up[v * (v < 100) + 100 * (v >= 100)]; // approximately 4 times faster than the Dennis solution and 8 times as fast as the original
  #elif defined(SOLUTION_2)
    if (v < (sizeof (look_up) / sizeof (uint8_t)))
      return look_up[v];
    return DEFAULT_BUCKET;
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
