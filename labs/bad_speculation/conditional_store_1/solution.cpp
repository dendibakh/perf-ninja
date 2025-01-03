
#include "solution.h"
#include "stdio.h"

uint8_t trailing_zero_bits_cnt(uint8_t mask){
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
  uint8_t mask =0;
  for(std::size_t i= 0 ; i < input.size(); i+=8){
    mask = (((lower <= input[i+0].first) && (input[i+0].first <= upper)) << 0) |
           (((lower <= input[i+1].first) && (input[i+1].first <= upper)) << 1) |
           (((lower <= input[i+2].first) && (input[i+2].first <= upper)) << 2) |
           (((lower <= input[i+3].first) && (input[i+3].first <= upper)) << 3) |
           (((lower <= input[i+4].first) && (input[i+4].first <= upper)) << 4) |
           (((lower <= input[i+5].first) && (input[i+5].first <= upper)) << 5) |
           (((lower <= input[i+6].first) && (input[i+6].first <= upper)) << 6) |
           (((lower <= input[i+7].first) && (input[i+7].first <= upper)) << 7);
    while(mask){
      uint8_t pos = trailing_zero_bits_cnt(mask);
      output[count++] = input[i+pos];
      mask = mask & ~(1 << pos);
    }

  }
  return count;
}
#endif


