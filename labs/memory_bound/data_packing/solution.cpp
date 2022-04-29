
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
