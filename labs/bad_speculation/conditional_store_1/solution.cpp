
#include "solution.h"
#include "stdio.h"

uint16_t trailing_zero_bits_cnt(uint16_t mask){
  int r = __builtin_ctzs(mask);
  return r;
}

#define SOLUTION
#ifndef SOLUTION
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
#else
// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  S it;
  for (const auto item : input) {
    uint32_t c = ((lower <= item.first) && (item.first <= upper));
    if (c) {
      it = item;
    }
    output[count] = it;
    count+=c;
  }
  return count;
}

#endif


