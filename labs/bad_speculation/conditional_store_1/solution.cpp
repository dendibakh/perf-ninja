
#include "solution.h"

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  const std::uint32_t d = upper - lower;
  for (const auto item : input) {
    output[count++] = item;
    count -= ((lower > item.first) || (item.first > upper));
  }
  return count;
}
