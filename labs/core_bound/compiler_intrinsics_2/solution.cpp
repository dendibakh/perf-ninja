#include "solution.hpp"

#define SOLUTION
#if defined(SOLUTION) && defined(__AVX2__)

#include <immintrin.h>
#include <iostream>
#include <cassert>

unsigned solution(const std::string &inputContents)
{
  constexpr int avx_char_num = 256 / 8;

  unsigned longestLine = 0;
  unsigned curLineLength = 0;

  int i = 0;
  const int char_num = inputContents.size();

  const __m256i newline = _mm256_set1_epi8('\n');
  const char *chars_in = inputContents.data();
  for (; i < char_num - avx_char_num; i += avx_char_num)
  {
    const __m256i chars = _mm256_loadu_si256((__m256i *)(chars_in + i));
    const __m256i cmp_mask256 = _mm256_cmpeq_epi8(chars, newline);
    int cmp_mask = _mm256_movemask_epi8(cmp_mask256);

    int internal_pos = 0;

    while (cmp_mask)
    {
      const unsigned int leading_zeros = _tzcnt_u32(cmp_mask);
      curLineLength += leading_zeros;
      longestLine = std::max(curLineLength, longestLine);
      curLineLength = 0;
      internal_pos += leading_zeros + 1;
      if (internal_pos > 31)
      {
        // 32 >= shifts are UB (also that means we processed the entire word)
        break;
      }
      cmp_mask >>= leading_zeros + 1;
    }

    curLineLength += 32 - internal_pos;
  }
  longestLine = std::max(curLineLength, longestLine);

  for (; i < char_num; ++i)
  {
    auto s = inputContents[i];
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}

#else
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
unsigned solution(const std::string &inputContents)
{
  unsigned longestLine = 0;
  unsigned curLineLength = 0;

  for (auto s : inputContents)
  {
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
#endif
