
#include "solution.h"
#include <algorithm>

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input, const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;

  for (const S &item : input) {
    const int result = (lower <= item.first) && (item.first <= upper);
    output[count] = item;
    count += result;
  }

  return count;
}
