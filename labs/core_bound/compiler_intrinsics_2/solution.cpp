#include "solution.hpp"

#include <cstdint>
#include <immintrin.h>

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

  uint32_t trailingLength = 0;
  uint32_t maxLineLength = 0;

  auto it = inputContents.begin();
  static constexpr int chunkSize = sizeof(__m256i);
  static const __m256i mask = _mm256_set1_epi8('\n');

  for (; it + chunkSize <= inputContents.end(); it += chunkSize) {

    __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it.base()));
    uint32_t newlines = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, mask));

    uint32_t nextTrailingLength = _lzcnt_u32(newlines);
    uint32_t lineBeginIdx = 0;

    while (newlines != 0) {

      uint32_t lineEndIdx = _tzcnt_u32(newlines);
      uint32_t length = lineEndIdx - lineBeginIdx;
      lineBeginIdx = lineEndIdx + 1;

      maxLineLength = std::max(trailingLength + length, maxLineLength);
      trailingLength = 0;
      newlines = _blsr_u32(newlines);
    }

    trailingLength += nextTrailingLength;
  }

  for (; it < inputContents.end(); ++it) {
    trailingLength = *it == '\n' ? 0 : trailingLength + 1;
    maxLineLength = std::max(trailingLength, maxLineLength);
  }

  return maxLineLength;
}
