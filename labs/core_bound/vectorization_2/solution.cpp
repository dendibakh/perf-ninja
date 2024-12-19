#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob) {
    acc += value;
  }
  /*
  * acc >> 16 -> shift bits 16 to the right
  * i.e. 0x12345678 -> 0x1234
  * 0xFFFF in hexdecimal -> 16 bits filled with one
  * in 32 bits that is 0000000000000000 1111111111111111
  * This means we affectively taking the last 16 bits with the bitwise or.
  * acc >> 16 is the upper 16 bits. If the acc after the + is still over larger than 16 bits
  * we continue this process (until nothing is left in the left part of the 32 bits)
  */
  while (acc >> 16) {
    acc = (acc & 0xFFFF) + (acc >> 16);
  }
  uint16_t acc_ = static_cast<uint16_t>(acc);

  return acc_;
}
