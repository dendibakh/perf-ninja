
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

#define SOLUTION

// __attribute__ ((always_inline))
static bool compare_ref(S const & a, S const & b)
{
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
#ifdef SOLUTION
  std::sort(arr.begin(), arr.end(), compare_ref);
#else
  qsort(arr.data(), arr.size(), sizeof(S), compare);
#endif
}

