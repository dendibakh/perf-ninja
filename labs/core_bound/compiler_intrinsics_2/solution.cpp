#include "solution.hpp"
#include <iostream>
#include <immintrin.h>
#include <cstdint>

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

  const char* const cursor = inputContents.c_str();
  const __m128i mask = _mm_set1_epi8('\n');
  for (size_t i = 0, size = inputContents.size(); i < size; ++i) {
    if ((i + 16) <= size)
    {
      const __m128i chunk = _mm_loadu_si128((const __m128i_u*)&cursor[i]);
      const __m128i result = _mm_cmpeq_epi8(chunk, mask);
      uint32_t matches = _mm_movemask_epi8(result);
      unsigned leading = _tzcnt_u32(matches);
      unsigned remaining = 16;
      while (leading != 32)
      {
        curLineLength += leading;
        longestLine = std::max(curLineLength, longestLine);
        const unsigned chop = leading + 1;
        remaining -= chop;
        matches >>= chop;
        curLineLength = 0;
        leading = _tzcnt_u32(matches);
      }
      curLineLength += remaining;
      longestLine = std::max(curLineLength, longestLine);
      i += 15;
    }
    else
    {
      curLineLength = (cursor[i] == '\n') ? 0 : curLineLength + 1;
      longestLine = std::max(curLineLength, longestLine);
    }
  }

  return longestLine;
}
