
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

#define SOLUTION
#ifdef SOLUTION

static int compare(const S &a, const S &b) {

  if (a.key1 < b.key1)
    return true;

  if (a.key1 > b.key1)
    return false;

  if (a.key2 < b.key2)
    return true;

  if (a.key2 > b.key2)
    return false;

  return false;
}

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), compare);
}

#else

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

void solution(std::array<S, N> &arr) {
  qsort(arr.data(), arr.size(), sizeof(S), compare);
}

#endif
