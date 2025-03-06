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

  // 2. counting sort
  constexpr int cntSize = maxRandom - minRandom + 1;
  std::array<short, cntSize> cnt{};
  static_assert(N <= std::numeric_limits<decltype(cnt)::value_type>::max());
  for (const auto& v : arr) {
    ++cnt[v.i - minRandom + 1];
  }
  for (int i = 1; i < cntSize; ++i) {
    cnt[i] += cnt[i - 1];
  }
  std::array<S, N> sorted;
  for (const auto& v : arr) {
    sorted[cnt[v.i - minRandom]++] = v;
  }
  arr = sorted;
}
