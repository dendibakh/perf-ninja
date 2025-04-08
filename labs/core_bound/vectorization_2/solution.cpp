#include "solution.hpp"
#include <array>

uint16_t checksum(const Blob &blob) {
  uint32_t acc { 0 }; // 32-bit accumulator to avoid overflow

  #pragma GCC ivdep
  #pragma omp for 
  for (auto value : blob) { 
    acc += value;
  }

  // exctract overflow (sum above 0xFFFF) with acc >> 16
  // retain the lower 16 bits with acc & 0xFFFF
  acc = ((acc & 0xFFFF) + (acc >> 16));
  // incase the previous addition caused overflow
  acc += (acc >> 16);

  return static_cast<uint16_t>(acc);
}