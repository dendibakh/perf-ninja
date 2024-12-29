#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc32 = 0;
  for (auto value : blob) {
    acc32 += value;
  }

  while ((acc32 >> 16) != 0){
    acc32 = (acc32 & 0xFFFF) + (acc32 >> 16);
  }
  return acc32;
}
