#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  const uint16_t val = acc & 0xFFFF;
  const uint16_t carry = acc >> 16;
  return val + carry;
}
