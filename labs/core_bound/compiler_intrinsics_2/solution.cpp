#include "solution.hpp"
#include <iostream>
#include <immintrin.h>
#include <cstdint>
#include <string.h>

// Find the longest line in a file.
// Implementation uses ternary operator with a hope that compiler will
// turn it into a CMOV instruction.
// The code inside the inner loop is equivalent to:
/*
if (s == '\n') {
  longestLine = std::max(curLineLength, longestLine);
  curLineLength = 0;
} else {
  curLineLength++;
}*/
unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;
  unsigned N = inputContents.size();

  static constexpr int kBatch = 16;

  auto newline_broadcast = _mm_set1_epi8('\n');

  int i = 0;
  for (; i + kBatch - 1 < N; i += kBatch) {
    auto x = _mm_loadu_si128((__m128i*)(inputContents.data() + i));
    auto mask = _mm_cmpeq_epi8(x, newline_broadcast);
    uint intmask = _mm_movemask_epi8(mask);
    int bits = kBatch;
    while (intmask != 0) {
      auto trailing_zeros = _tzcnt_u32(intmask);
      curLineLength += trailing_zeros;
      longestLine = std::max(curLineLength, longestLine);
      intmask >>= trailing_zeros + 1;
      bits -= trailing_zeros + 1;
      curLineLength = 0;
    }
    curLineLength += bits;
  }

  for (; i < N; i++) {
    curLineLength = (inputContents[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  } 

  return longestLine;
}
