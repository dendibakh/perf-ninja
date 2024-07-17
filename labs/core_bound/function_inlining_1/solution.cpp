
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



void quick_sort(std::array<S, N> &arr, int start, int end) {
  if (start >= end)
    return;

  // reorder
  int K = start;
  auto& flag = arr[end];
  for (int i = start; i < end; ++i) {
    auto& curr = arr[i];
    if (curr.key1 < flag.key1 || (curr.key1 == flag.key1 && curr.key2 < flag.key2)) {
      if (i != K)
        std::swap(arr[K], arr[i]);
      ++K;
    }
  }
  std::swap(arr[K], arr[end]);

  quick_sort(arr, start, K - 1);
  quick_sort(arr, K + 1, end);
}

void solution(std::array<S, N> &arr) {
  // qsort(arr.data(), arr.size(), sizeof(S), compare);
  quick_sort(arr, 0, arr.size()-1);
}
