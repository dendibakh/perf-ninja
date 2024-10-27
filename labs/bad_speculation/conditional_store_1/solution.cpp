
#include "solution.h"

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  bool mask = false;
  for (const auto& item : input) {
    mask = (lower <= item.first) & (item.first <= upper);
    output[count] = item;
    count += mask;
  }
  return count;
}
