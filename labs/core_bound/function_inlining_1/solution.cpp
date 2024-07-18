
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

void solution(std::array<S, N> &arr)
{
  std::sort(arr.begin(), arr.end(), [](const S& a, const S& b)
  {
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  });
}