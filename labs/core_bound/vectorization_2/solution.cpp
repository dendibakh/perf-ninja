#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  uint32_t acc32 = 0;
  for (auto value : blob) {
    acc32 += value;
  }

  acc = acc32 & 0xFFFF;
  acc += acc32 >> 16;
  return acc;
}
