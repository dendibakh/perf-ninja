
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

static bool compare(const S& a, const S& b) {
  if (a.key1 < b.key1)
    return true;

  if (a.key1 > b.key1)
    return false;

  if (a.key2 < b.key2)
    return true;

  return false;
}

void solution(std::array<S, N> &arr) {
  std::sort(arr.data(), arr.data() + arr.size(), [](const S& a, const S& b) {
    return compare(a, b);
  });
}
