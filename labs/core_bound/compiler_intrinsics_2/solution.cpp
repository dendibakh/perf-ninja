#include "solution.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>
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
unsigned solution(const std::string& inputContents) {
    unsigned longestLine = 0;
    unsigned curLineLength = 0;

    size_t index = 0;
    const size_t n = inputContents.size();

    const __m128i nl = _mm_set1_epi8('\n');

    for (; index + 16 <= n; index += 16) {
        const char* ptr = inputContents.data() + index;

        __m128i chunk = _mm_loadu_si128(
            reinterpret_cast<const __m128i*>(ptr));

        __m128i cmp = _mm_cmpeq_epi8(chunk, nl);
        unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(cmp));

        if (mask == 0) {
            curLineLength += 16;
            continue;
        }

        int prev = 0;

        while (mask != 0) {
            unsigned pos = __builtin_ctz(mask);

            curLineLength += (pos - prev);
            longestLine = std::max(longestLine, curLineLength);
            curLineLength = 0;

            prev = pos + 1;
            mask &= (mask - 1);
        }

        curLineLength += (16 - prev);
    }

    // tail
    for (; index < n; ++index) {
        if (inputContents[index] == '\n') {
            longestLine = std::max(longestLine, curLineLength);
            curLineLength = 0;
        } else {
            ++curLineLength;
        }
    }

    longestLine = std::max(longestLine, curLineLength);
    return longestLine;
}
