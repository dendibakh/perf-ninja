#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  uint16_t low  = acc & 0xFFFF;      // last 16 bits
  uint16_t high = acc >> 16;         // first 16 bits
  return low + high + (low + high > UINT16_MAX ? 1 : 0);
}
