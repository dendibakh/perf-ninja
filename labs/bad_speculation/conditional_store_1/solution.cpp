
#include "solution.h"

#include <algorithm>
#include <iterator>
#include <iostream>

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::cout << "lower: " << lower << ", upper: " << upper << std::endl;
  const auto begin = std::lower_bound(
    std::cbegin(input),
    std::cend(input),
    S{lower, 0});
  const auto end = std::upper_bound(
    begin,
    std::cend(input),
    S{upper,0});
  
  std::copy(begin, end, std::begin(output));
  return std::distance(begin, end);
}
