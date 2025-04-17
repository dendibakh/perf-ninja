// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "solution.h"
#include <algorithm>
#include <array>
#include <random>

void solution(std::vector<S> &arr) {
  // 1. shuffle
  static std::random_device rd;
  static std::mt19937 g(rd());
  std::shuffle(arr.begin(), arr.end(), g);

  // 2. counting sort
  constexpr int cntSize = maxRandom - minRandom + 1;
  std::array<int, cntSize> cnt{};
  for (const auto& v : arr) {
    ++cnt[v.i - minRandom + 1];
  }
  for (int i = 1; i < cntSize; ++i) {
    cnt[i] += cnt[i - 1];
  }
  std::vector<S> sorted(N);
  for (const auto& v : arr) {
    sorted[cnt[v.i - minRandom]++] = v;
  }
  arr = sorted;
}
