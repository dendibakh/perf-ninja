
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
  uint16_t mask =0;
  for(std::size_t i= 0 ; i < input.size(); i+=16){
    mask = (((lower <= input[i+0].first) && (input[i+0].first <= upper)) << 0) |
           (((lower <= input[i+1].first) && (input[i+1].first <= upper)) << 1) |
           (((lower <= input[i+2].first) && (input[i+2].first <= upper)) << 2) |
           (((lower <= input[i+3].first) && (input[i+3].first <= upper)) << 3) |
           (((lower <= input[i+4].first) && (input[i+4].first <= upper)) << 4) |
           (((lower <= input[i+5].first) && (input[i+5].first <= upper)) << 5) |
           (((lower <= input[i+6].first) && (input[i+6].first <= upper)) << 6) |
           (((lower <= input[i+7].first) && (input[i+7].first <= upper)) << 7) |
           (((lower <= input[i+8].first) && (input[i+8].first <= upper)) << 8) |
           (((lower <= input[i+9].first) && (input[i+9].first <= upper)) << 9) |
           (((lower <= input[i+10].first) && (input[i+10].first <= upper)) << 10) |
           (((lower <= input[i+11].first) && (input[i+11].first <= upper)) << 11) |
           (((lower <= input[i+12].first) && (input[i+12].first <= upper)) << 12) |
           (((lower <= input[i+13].first) && (input[i+13].first <= upper)) << 13) |
           (((lower <= input[i+14].first) && (input[i+14].first <= upper)) << 14) |
           (((lower <= input[i+15].first) && (input[i+15].first <= upper)) << 15);
    while(mask){
      uint16_t pos = trailing_zero_bits_cnt(mask);
      output[count++] = input[i+pos];
      mask = mask & ~(1 << pos);
    }

  }
  return count;
}
#endif


