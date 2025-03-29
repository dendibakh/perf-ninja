#include "solution.h"
#include <algorithm>
#include <random>
#include <array>

void solution(std::array<S, N>& arr) {
  // 1. Shuffle the array
  static std::random_device rd;
  static std::mt19937 g(rd());
  std::shuffle(arr.begin(), arr.end(), g);

  // 2. Counting sort without prefix sums
  constexpr int cntSize = maxRandom - minRandom + 1;
  std::array<int, cntSize> freq{};  // Frequency array initialized to 0

  // Count the frequency of each value
  for (const auto& v : arr) {
    ++freq[v.i - minRandom];
  }

  // Populate the sorted array directly using frequencies
  int k = 0;  // Index in the sorted array
  for (int value = 0; value < cntSize; ++value) {
    for (int count = 0; count < freq[value]; ++count) {
      arr[k++] = S{value + minRandom};
    }
  }
}