#include "solution.hpp"
#include <cassert>

uint16_t checksum(const Blob &blob) {

#ifdef SOLUTION
  assert(blob.size() <= (1 << 16));
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  uint16_t top = (acc >> 16);
  uint16_t bottom = (acc & 0xFFFF);
  uint16_t res = top + bottom;
  res += res < top;
  return res;
#else
  uint16_t acc = 0;
  for (auto value : blob) {
    acc += value;
    acc += acc < value; // add carry
  }
  return acc;
#endif

}
