#include "solution.hpp"
#include <iostream>
#include <cstdint>
#include "immintrin.h"

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
  size_t i = 0;
  size_t sz = inputContents.size();
  const auto* ptr = inputContents.data();
  auto newline_vector = _mm256_set1_epi8('\n');
  for (; i + 32 <= sz; i += 32) {
    auto chunk = _mm256_loadu_si256((const __m256i*)(ptr + i));
    auto mask = _mm256_cmpeq_epi8(chunk, newline_vector);
    uint32_t bitmask = _mm256_movemask_epi8(mask); // convert 256-bit mask to 32-bit integer
    if (bitmask == 0){
      curLineLength += 32; // No newlines in this chunk, add 32 to current line length
      continue;
    }
    int prev_index = -1; // index of the previous newline character
    while (bitmask) {
      auto index = __builtin_ctz(bitmask); // TRAILING because Intel has little-endian int
      curLineLength += index - prev_index - 1;
      longestLine = std::max(curLineLength, longestLine);
      curLineLength = 0;
      prev_index = index;
      bitmask &= bitmask - 1;
    }
    curLineLength += 31 - prev_index; // Add length of last segment after the last newline in the chunk
  }

  for (; i < sz; i++) {
    curLineLength = (inputContents[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
