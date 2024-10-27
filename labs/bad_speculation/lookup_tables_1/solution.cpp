#include "solution.hpp"

constexpr std::array<std::size_t, 151> bucket_map {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-12
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13-28
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 29-40
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,// 41-52
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 53-70
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // 71-82
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 83-99
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // 100-109
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7
};

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  
  #pragma unroll
  for (std::size_t i = 0; i < values.size(); i++) {
    retBuckets[bucket_map[values[i]]]++;
  }
  
  return retBuckets;
}
