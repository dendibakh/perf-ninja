#include "solution.hpp"
#include <iostream>
#include <immintrin.h>
#include <emmintrin.h>

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
  int i{};
  __m256i eol{_mm256_set1_epi8('\n')};
  for (; i + 31 < inputContents.size(); i += 32) {
	  // Load next 256 bit (32 chars) segment of input
	  __m256i segment{_mm256_loadu_si256((__m256i const*)&inputContents[i])};

	  // Compare with eol: results in 0xFF for match, otherwise 0x00
	  __m256i match{_mm256_cmpeq_epi8(segment, eol)};

	  // Convert to int, using one bit from each byte of match
	  unsigned matchmask = _mm256_movemask_epi8(match);

	  // Find trailing match
	  unsigned matchi{};
	  while (matchi < 32) {
	  	unsigned n_tz{_tzcnt_u32(matchmask)};
		unsigned matchj{std::min(32U, matchi + n_tz)};
		longestLine = std::max(longestLine, curLineLength += matchj - matchi);
		if (matchj < 32) curLineLength = 0;
		matchi = matchj + 1;
		matchmask >>= (n_tz + 1);
	  }
  }

  for (; i < inputContents.size(); ++i) {
    curLineLength = (inputContents[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
