#include "solution.hpp"
#include <iostream>
#include <immintrin.h>

// Find the longest line in a file using x86 intrinsics.
uint32_t solution(const std::string &inputContents) {
  uint32_t longestLine = 0;
  unsigned curLineLength = 0;
  uint32_t pos = 0;
  auto strLength = inputContents.size();
  if (strLength >= 32) { // size of YMM vector
    const __m256i eol = _mm256_set1_epi8('\n');
    auto *buffer = inputContents.data();
    uint32_t curLineBegin = 0;
    for (; pos + 32 < strLength; pos += 32) {
      // Load the next 32-byte chunk of the input string.
      __m256i vect = _mm256_loadu_si256((const __m256i *)buffer);
      // Check every character in this chunk. Get the result of the
      // comparison back as a vector mask.
      __m256i vectMask = _mm256_cmpeq_epi8(vect, eol);
      // Convert the vector mask into the 32-bit integer representation.
      uint32_t mask = _mm256_movemask_epi8(vectMask);
      while (mask) {
        // Find the position of the trailing '1' bit.
        // Requires BMI1 x86-ISA extention
        int maskPos = _tzcnt_u32(mask);

        // Compute the length of the current string.
        uint32_t curLineLength = (pos - curLineBegin) + maskPos;
        if (pos < curLineBegin)
          curLineLength = maskPos;

        // Set the beginning of the next line to the next character after the
        // line separator
        curLineBegin += curLineLength + 1;

        // Is this line the longest?
        longestLine = std::max(curLineLength, longestLine);

        // Shift the mask to check if we have more line separators
        ++maskPos;
        if (maskPos > 31) // Shifting 32-bit integer by 32 positions is an UB.
          break;
        else
          mask >>= maskPos;
      }
      // Advance the pointer to the next 32-byte input chunk.
      buffer += 32;
    }
    // Calculate the length of the current line.
    curLineLength = pos - curLineBegin;
  }
  // This loop processes the remaining characters sequentially.
  for (; pos < strLength; pos++) {
    if (inputContents[pos] == '\n') {
      longestLine = std::max(curLineLength, longestLine);
      curLineLength = 0;
    } else {
      curLineLength++;
    }
  }
  // if no end-of-line in the end
  longestLine = std::max(curLineLength, longestLine);
  return longestLine;
}
