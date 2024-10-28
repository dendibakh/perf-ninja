
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), [](const S &a, const S &b) {
    return std::tuple(a.key1, a.key2) < std::tuple(b.key1, b.key2);
  });
}
