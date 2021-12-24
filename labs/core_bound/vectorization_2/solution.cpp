#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;

  for (auto value : blob) {
    acc += value;
  }
  
  const uint16_t acc16bits = acc;
  const uint16_t carry = (acc >> 16u);
  const uint16_t result = acc16bits + carry;

  return result + (result < carry);
}
