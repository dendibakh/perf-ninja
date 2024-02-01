#include "solution.hpp"

template<int N>
struct LookUpTable {
  constexpr LookUpTable() : arr() {
    int i = 0;
    unsigned int v = 0;
    for (; i < N; ++i) {
      v += (i == 13 || i == 29 || i == 41 || i == 53 || i == 71 || i == 83);
      arr[i] = v; 
    }
  }
  std::array<std::size_t, N> arr;
};
constexpr auto look_up_table = LookUpTable<100>();

static std::size_t mapToBucket(std::size_t v) {    // diff
  /*if (v >= 0 && v < 13)        return 0; // 13
  else if (v >= 13 && v < 29)  return 1; // 16
  else if (v >= 29 && v < 41)  return 2; // 12
  else if (v >= 41 && v < 53)  return 3; // 12
  else if (v >= 53 && v < 71)  return 4; // 18
  else if (v >= 71 && v < 83)  return 5; // 12
  else if (v >= 83 && v < 100) return 6; // 17
  return -1;  // let it crash */
  return (v < look_up_table.arr.size()) ? look_up_table.arr[v] : -1;
  //return v >= 100 ? -1 :
  //  ((v >= 13) + (v >= 29) + (v >= 41) + (v >= 53) + (v >= 71) + (v >= 83));
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
