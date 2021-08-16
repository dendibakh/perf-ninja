
#include "solution.h"
#include <algorithm>
#include <stdlib.h>
#include <tuple>

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), [](const S &a, const S &b) {
    return std::tie(a.key1, a.key2) < std::tie(b.key1, b.key2);
  });
}
