#include "solution.hpp"
#include <limits.h>

/**
 * in order to do the sum first and the carry later in one go, enche making the loop vectorizable,
 * one uses a larger accumulator. The times in which the 16bits accumulator would overflow
 * will then be codified in the leftmost 16 bits. Once summed the accumulator is split in 2:
 * the leftmost part, in which the overflows are codified, and the lower part, which is identical
 * to what the 16 bits accumulator would have been. The 2 parts are then summed together, and since this 
 * could still leads to a 16 bits overflow the procedure is repeated befor casting the accumulator to 16 bits 
*/

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  constexpr std::uint32_t lowerBits{0x0000FFFF}; // this mask zeroes the upper 16 bits

  for (auto value : blob) {
    acc += value;
  }

  auto high = acc>>16;
  auto low = acc & lowerBits;
  acc = low+high;
  high = acc>>16;
  low = acc & lowerBits;

  return static_cast<uint16_t>(low+high);
}
