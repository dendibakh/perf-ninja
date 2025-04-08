
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

static int compare(const void *lhs, const void *rhs) {
  auto &a = *reinterpret_cast<const S *>(lhs);
  auto &b = *reinterpret_cast<const S *>(rhs);

  if (a.key1 < b.key1)
    return -1;

  if (a.key1 > b.key1)
    return 1;

  if (a.key2 < b.key2)
    return -1;

  if (a.key2 > b.key2)
    return 1;

  return 0;
}

inline static bool compare_bool(const S& a, const S& b) {
  if (a.key1 != b.key1)
    return a.key1 < b.key1;
  return a.key2 < b.key2;
}

void solution(std::array<S, N> &arr) {
  // qsort(arr.data(), arr.size(), sizeof(S), compare);
  
  // std::sort(arr.begin(), arr.end(), compare_bool); // 360s

  //  dendi solution // 290us
  // std::sort(arr.begin(), arr.end(), [](S& a, S& b){
  //   return a.key1 < b.key1 || (a.key1 == b.key1) && (a.key2 < b.key2);
  // });

  // fastest 266 us
  std::sort(arr.begin(), arr.end(), [](S& a, S& b){
    if (a.key1 != b.key1)
      return a.key1 < b.key1;
    return a.key2 < b.key2;
  });
  
}
