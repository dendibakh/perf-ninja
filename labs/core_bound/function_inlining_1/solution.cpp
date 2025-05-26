
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
#define SOLUTION
void solution(std::array<S, N> &arr) {
#ifdef SOLUTION
  std::sort(arr.begin(), arr.end(),
  [](const S &a, const S &b) {
    if (a.key1 < b.key1)   return true;
    if (a.key1 > b.key1)   return false;
    // key1 equal ⇒ compare key2
    if (a.key2 < b.key2)   return true;
    if (a.key2 > b.key2)   return false;
    // key1 == key2 ⇒ neither is less
    return false;
  }
  );
#else
  qsort(arr.data(), arr.size(), sizeof(S), compare);
#endif

}
