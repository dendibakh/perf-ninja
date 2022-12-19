
#include "solution.h"
int MinBitHacks(int x, int y) {
 return y + ((x - y) & ((x - y) >> 31));
}
 
int MaxBitHacks(int x, int y) {
  return x - ((x - y) & ((x - y) >> 31));
}

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto item : input) {
    
    const int low = MaxBitHacks(item.first, lower);
    const int high = MinBitHacks(item.first, upper);
    output[count] = item;
    // We only increment count if the item is in range
    count += (low == high);
  }
  return count;
}
