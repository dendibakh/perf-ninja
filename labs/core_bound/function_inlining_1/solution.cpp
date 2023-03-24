#include "solution.h"
#include <algorithm>

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), [](S &a, S &b) {
    return a.key1 < b.key1 || (a.key1 == b.key1) && (a.key2 < b.key2);
  });
}
