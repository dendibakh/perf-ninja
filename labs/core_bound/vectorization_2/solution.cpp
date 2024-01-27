#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  uint16_t c = acc >> 16;
  uint16_t res = acc & 0xFFFF;
  res += c;
  res += res < c;
  return res;
}