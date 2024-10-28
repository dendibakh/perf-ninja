#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc32 = 0;
  for (int i = 0; i < blob.size(); ++i) {
    acc32 += blob[i];
  }

  uint16_t acc = (uint16_t)(acc32 & 0xFFFF) + (uint16_t)(acc32 / 0xFFFF);
  return acc;
}
