#include "solution.hpp"
#include <iostream>

#include <emmintrin.h>
#include <immintrin.h>

// Find the longest line in a file.

#ifndef SOLUTION
  #define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

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

  for (auto s : inputContents) {
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}

#elif SOLUTION == 1
// My solution.

unsigned solution(const std::string& string)
{
  // Get a SIMD view of the string data.
  __m128i const* const data = reinterpret_cast<const __m128i*>(string.data());
  size_t const numBlocks = string.length() / 16;

  __m128i const newlines = _mm_set1_epi8('\n');

  // State we update as we walk across the blocks of the string.
  unsigned longestLine {0};
  unsigned curLineLength {0};

  // Walk across the blocks of the string.
  for (size_t blkIdx = 0; blkIdx < numBlocks; ++blkIdx)
  {
    // Read the current block.
    __m128i const* const blockStart = data + blkIdx;
    __m128i const block = _mm_loadu_si128(blockStart);

    // Generate mask.
    __m128i eqByteMask = _mm_cmpeq_epi8(block, newlines);
    int eqBitMask = _mm_movemask_epi8(eqByteMask);

    // Find the lengths of the strings in this block.
    // The intent of 'consumed' is to keep track of how many
    // characters wasn't used and should be carried over to
    // the next block.
    int consumed {0};
    if (eqBitMask == 0)
    {
      // No newlines in this block, carry over all
      // 16 characters to the next block.
      curLineLength += 16;
      continue;
    }

    // At least one newline. Find string lengths within this block.
    while (eqBitMask != 0)
    {
      // Find the length of the next string-part that is in this block.
      // Add it to any characters that was carried over from the previous
      // block.
      unsigned int const newlineIdx = _tzcnt_u32(eqBitMask);
      curLineLength += newlineIdx;
      consumed += newlineIdx + 1;
      longestLine = std::max(curLineLength, longestLine);

      // Done with that line, start a new one.
      curLineLength = 0;
      eqBitMask >>= (newlineIdx + 1);

      // No more complete lines in this block, find how many characters
      // wasn't used and carry that over to the next block.
      if (eqBitMask == 0)
      {
        curLineLength = 16 - consumed;
      }
    }
  }

  if (string.length() % 16 != 0)
  {
    // Handle partial block at the end of the string.
    for (size_t i = 16 * numBlocks; i < string.length(); ++i)
    {
      char const s = string[i];
      curLineLength = (s == '\n') ? 0 : curLineLength + 1;
      longestLine = std::max(curLineLength, longestLine);
    }
  }
  else
  {
    // There was no partial block, so finalize the
    // carry-over from the last complete block.
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}

#elif SOLUTION == 2
uint32_t solution(const std::string &inputContents) {
  uint32_t longestLine = 0;
  uint32_t curLineLength = 0;
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
    curLineLength = (inputContents[pos] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }
  return longestLine;
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif
