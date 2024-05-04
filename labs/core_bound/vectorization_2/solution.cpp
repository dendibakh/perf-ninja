#include "solution.hpp"

#include <cstring>

#ifndef SOLUTION
  #define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  for (auto value : blob) {
    acc += value;
    acc += acc < value; // add carry
  }
  return acc;
}


#elif SOLUTION == 1
// My solution.


// Variant of facit where the accumulation is done on 32-bit words into a 64-bit
// accumulator.
//
// Doesn't work, validateLab fails from time to time.
uint16_t checksum(const Blob& blob) {
  uint64_t acc64 = 0;
  const uint32_t* blob32 = reinterpret_cast<const uint32_t*>(&blob[0]);
  for (uint64_t i = 0; i < N / 2; ++i) {
    acc64 += blob32[i];
  }

  // We now have four 16-bit values in acc.
  // Sum them pair-wise using facit's 32-bit summation code.

  uint32_t acc32 {0};

  // Lower 32-bits.
  {
    uint32_t acc = acc64 & 0xFFFFFFFFu;

    // sum up two halves of uint32
    auto high = acc >> 16;
    auto low = acc & 0xFFFFu;
    acc = low + high;

    // account for potential overflow.
    high = acc >> 16;
    low = acc & 0xFFFFu;
    acc32 += low + high;
  }

  // Upper 32-bits
  {
    uint32_t acc = acc64 >> 32;

    // sum up two halves of uint32
    auto high = acc >> 16;
    auto low = acc & 0xFFFFu;
    acc = low + high;

    // account for potential overflow.
    high = acc >> 16;
    low = acc & 0xFFFFu;
    acc32 += low + high;
  }

  return static_cast<uint16_t>(acc32);
}


// First attempt, based on the "Parallel Summation" section of RFC 1071.
#if 0
uint16_t checksum(const Blob &blob) {
  constexpr uint64_t n {8};
  using Block64 = std::array<uint64_t, n>;
  constexpr uint64_t BlockSize = sizeof(Block64);
  constexpr uint64_t elemsPerBlock = BlockSize / sizeof(uint16_t);
  using Block16 = std::array<uint16_t, elemsPerBlock>;
  static_assert(sizeof(Block16) == BlockSize);

  Block64 accumulator64 = {0};
  Block64 overflows = {0};

  for (uint64_t i = 0; i < N; i += elemsPerBlock)
  {
    Block64 elems;
    memcpy(&elems[0], &blob[i], sizeof(Block64));
    for (uint64_t e = 0; e < n; ++e)
    {
      accumulator64[e] += elems[e];
      overflows[e] += accumulator64[e] < elems[e];
    }
  }

  for (uint64_t i = 0; i < n; ++i)
  {
    accumulator64[i] += overflows[i];
    accumulator64[i] += accumulator64[i] < overflows[i];
  }

  Block16 accumulator16;
  memcpy(&accumulator16[0], &accumulator64[0], BlockSize);

  uint16_t accumulator {0};
  for (uint64_t i = 0; i < elemsPerBlock; ++i)
  {
    accumulator += accumulator16[i];
    accumulator += accumulator < accumulator16[i];
  }

  return accumulator;
}
#endif

#elif SOLUTION == 2
// Facit.

uint16_t checksum(const Blob& blob) {
  uint32_t acc = 0;
  for (const auto value : blob) {
    acc += value;
  }

  // sum up two halves of uint32
  auto high = acc >> 16;
  auto low = acc & 0xFFFFu;
  acc = low + high;

  // account for potential overflow.
  high = acc >> 16;
  low = acc & 0xFFFFu;
  acc = low + high;

  return static_cast<uint16_t>(acc);
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif




