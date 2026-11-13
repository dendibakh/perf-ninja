
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

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1            657 us          657 us         1034
void solution_baseline(std::array<S, N> &arr) {
  qsort(arr.data(), arr.size(), sizeof(S), compare);
}

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1            428 us          428 us         1645
void solution(std::array<S, N> &arr) {
  std::sort(
      arr.begin(), 
      arr.end(), 
      [](const S& a, const S&b) { return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2); });
}