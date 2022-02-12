#include "solution.hpp"
#include <iostream>
#include <immintrin.h>
#include <emmintrin.h>
#include <bit>
#include <iostream>
#include <string_view>

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
  size_t pos = 0;

  auto pat = _mm256_set1_epi8('\n');
  for( ; pos + 32 < inputContents.size(); pos += 32) {
    auto str = _mm256_loadu_si256((const __m256i_u *)&inputContents[pos]);
    auto cmp_res = _mm256_cmpeq_epi8(str, pat);
    auto cmp_bits = static_cast<uint64_t>(static_cast<uint32_t>(_mm256_movemask_epi8(cmp_res)));
    
    if (!cmp_bits) {
      curLineLength += 32;
      continue;
    }

    auto num_bits_remaining = 32;
    do {
      auto num_trailing_zeroes = __builtin_ctz(cmp_bits);
      curLineLength += num_trailing_zeroes;
      longestLine = std::max(curLineLength, longestLine);
      cmp_bits = cmp_bits >> (num_trailing_zeroes + 1);
      num_bits_remaining -= num_trailing_zeroes + 1;
      curLineLength = 0;
    } while (cmp_bits);
    
    curLineLength = num_bits_remaining;
  }

  for (; pos < inputContents.size(); pos++) {
    auto s = inputContents[pos];
    longestLine = std::max(curLineLength, longestLine);
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
  }

  // if no end-of-line in the end
  longestLine = std::max(curLineLength, longestLine);
  return longestLine;
}
