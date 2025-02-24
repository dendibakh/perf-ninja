#include "solution.hpp"
#include <bit>
#include <x86intrin.h>

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

  constexpr int vec_sz = 16;
  const auto mask = _mm_set1_epi8('\n');
  int i = 0;
  for (; i + vec_sz <= inputContents.size(); i += vec_sz) {
    const auto d = _mm_loadu_si128((const __m128i*)&inputContents[i]);
    const auto cmp = _mm_cmpeq_epi8(d, mask);
    unsigned cmp_mask = _mm_movemask_epi8(cmp);
    int prev_idx = 0;
    while (cmp_mask) {
      const int idx = std::countr_zero(cmp_mask);
      curLineLength += idx - prev_idx;
      longestLine = std::max(curLineLength, longestLine);
      curLineLength = 0;
      cmp_mask = cmp_mask & (cmp_mask - 1);
      prev_idx = idx + 1;
    }
    curLineLength += vec_sz - prev_idx;
  }
  for (; i < inputContents.size(); ++i) {
    curLineLength = (inputContents[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
