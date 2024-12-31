#include "solution.hpp"

#define SOLUTION
#ifndef SOLUTION
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
#else //mine


static std::size_t mapToBucket(std::size_t v) {
  constexpr auto LUT = []{
    constexpr auto size = 100;
    std::array<size_t,size> l = {};
    for(int i=0; i < size; ++i){
        if      (i < 13)  l[i]= 0; //   13
        else if (i < 29)  l[i]= 1; //   16
        else if (i < 41)  l[i]= 2; //   12
        else if (i < 53)  l[i]= 3; //   12
        else if (i < 71)  l[i]= 4; //   18
        else if (i < 83)  l[i]= 5; //   12
        else if (i < 100) l[i]= 6; //   17
    }
    return l;
  }();
  static_assert(LUT[99] == 6);

  if (v >= 100){ return DEFAULT_BUCKET;}
  return LUT[v];
}
#endif

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
