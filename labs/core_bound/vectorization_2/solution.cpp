#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  uint16_t carry = 0;
  for (auto value : blob) {
    acc += value;
    carry += acc < value; // add carry
  }
  acc += carry;
  if (acc < carry)
  {
    acc += 1;
  }
  
  return acc;
}
