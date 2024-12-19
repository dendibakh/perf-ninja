#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  Carry carry = {};
  for (int i = 0; i < blob.size(); i++) {
    uint16_t value = blob[i];
    acc += value;
    carry[i] = acc < value; // save carry
  }
  for (int i = 0; i < blob.size(); i++) {
    acc += carry[i];
  }
  return acc;
}
