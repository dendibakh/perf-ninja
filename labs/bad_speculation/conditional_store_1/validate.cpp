
#include "solution.h"
#include <iostream>
#include <limits>

static std::size_t originalSelect(std::array<S, N> &output,
                                  const std::array<S, N> &input,
                                  const std::uint32_t lower,
                                  const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto item : input) {
    if ((lower <= item.first) && (item.first <= upper)) {
      output[count++] = item;
    }
  }
  return count;
}

static bool equals(const std::array<S, N> &a, const std::array<S, N> &b,
                   std::size_t size) {
  constexpr int maxErrors = 10;

  int errors = 0;
  for (std::size_t i = 0; i < size; i++) {
    auto va = a[i];
    auto vb = b[i];
    if (va != vb) {
      if (va.first != vb.first) {
        std::cerr << "Result[" << i << "].first = " << va.first << ". Expected["
                  << i << "].first = " << vb.first << std::endl;
      }
      if (va.second != vb.second) {
        std::cerr << "Result[" << i << "].second = " << va.second
                  << ". Expected[" << i << "].second = " << vb.second
                  << std::endl;
      }
      if (++errors >= maxErrors)
        return false;
    }
  }
  return 0 == errors;
}

int main() {
  std::array<S, N> arr;
  init(arr);

  constexpr auto lower = (std::numeric_limits<std::uint32_t>::max() / 4) + 1;
  constexpr auto upper =
      (std::numeric_limits<std::uint32_t>::max() / 2) + lower;

  std::array<S, N> expected, result;
  auto expectedSize = originalSelect(expected, arr, lower, upper);
  auto resultSize = select(result, arr, lower, upper);

  if (resultSize != expectedSize) {
    std::cerr << "Result size = " << resultSize
              << ". Expected size = " << expectedSize << std::endl;
    return 1;
  }

  if (!equals(result, expected, expectedSize)) {
    std::cerr << "Validation Failed" << std::endl;
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
