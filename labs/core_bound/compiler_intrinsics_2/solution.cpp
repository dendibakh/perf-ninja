#include "solution.hpp"
#include <immintrin.h>
#include <iostream>

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

  int sz = inputContents.size();
  const char* data = inputContents.data();

  __m256i backslash = _mm256_set1_epi8('\n');

  int i = 0;
  for (i = 0; i + 32 < sz;) {
    __m256i vec = _mm256_loadu_si256((__m256i*)(char*)&data[i]);
    int mask = _mm256_cmpeq_epi8_mask(vec, backslash);
    if (mask) {
      int x = __builtin_ctz(mask);
      longestLine = std::max(curLineLength + x, longestLine);
      curLineLength = 0;
      i += x + 1;
    } else {
      i += 32;
      curLineLength += 32;
      longestLine = std::max(curLineLength, longestLine);
    }
  }


  for (; i < sz; ++i) {
    curLineLength = (data[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
