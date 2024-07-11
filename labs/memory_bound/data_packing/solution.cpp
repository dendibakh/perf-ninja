// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "solution.h"
#include <algorithm>
#include <random>

void solution(std::array<S, N> &arr) {
  // 1. shuffle
  static std::random_device rd;
  static std::mt19937 g(rd());
  std::shuffle(arr.begin(), arr.end(), g);

  // 2. sort
  std::sort(arr.begin(), arr.end());
}
