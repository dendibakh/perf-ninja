
#include "solution.h"

#define SOLUTION

#ifdef SOLUTION
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto item : input) {
    uint32_t cmp = (lower <= item.first) && (item.first <= upper);
    S x{};
    if (__builtin_unpredictable(cmp)) {
        x = item;
    }
    output[count] = x;
    count += cmp;
  }
  return count;
}

#else
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto item : input) {
    if ((lower <= item.first) && (item.first <= upper)) {
      output[count++] = item;
    }
  }
  return count;
}
#endif
