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

  __m128i backslash = _mm_set1_epi8('\n');

  int i = 0;
  for (i = 0; i + 16 < sz;) {
    __m128i vec = _mm_loadu_si128((__m128i*)(char*)&data[i]);

    __m128i cmp_result = _mm_cmpeq_epi8(vec, backslash);
    int mask = _mm_movemask_epi8(cmp_result);
    if (mask) {
      int x = __builtin_ctz(mask);
      longestLine = std::max(curLineLength + x, longestLine);
      curLineLength = 0;
      i += x + 1;
    } else {
      i += 16;
      curLineLength += 16;
      longestLine = std::max(curLineLength, longestLine);
    }
  }


  for (; i < sz; ++i) {
    curLineLength = (data[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
