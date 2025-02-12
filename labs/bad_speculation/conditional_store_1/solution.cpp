
#include "solution.h"
#define SOLUTION
// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto item : input) 
  {
    #ifdef SOLUTION
    output[count] = item;
    count += static_cast<uint32_t>((lower <= item.first) && (item.first <= upper));
    #else
    if ((lower <= item.first) && (item.first <= upper)) {
      output[count++] = item;
    }
    #endif
  }
  return count;
}
