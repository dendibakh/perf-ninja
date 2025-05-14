#include "solution.hpp"
#include <cstdint>
#include <immintrin.h>
#include <iostream>
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
uint32_t solution(const std::string &inputContents)
{
    uint64_t longestLine{0};
    uint64_t curLineLength{0};
    uint64_t pos{0};
    auto strLength = inputContents.size();

    if (strLength >= 64)
    {                                               // size of YMM
        const __m512i eol = _mm512_set1_epi8('\n'); // fill with \n
        auto *buffer = inputContents.data();
        uint64_t curLineBegin{0};

        for (; pos + 64 < strLength; pos += 64)
        {
            // Load the next 32-byte chucnk of the input string

            __m512i vect = _mm512_loadu_si512(buffer);
            //_mm_prefetch(buffer + 64, _MM_HINT_NTA); // doesnt help, compiler is smart
            // compare vect to \n mask
            __mmask64 vectMask = _mm512_cmpeq_epi8_mask(vect, eol);
            // convert the vector mask into the 64-bit integer representation
            uint64_t mask = vectMask;
            while (mask)
            {
                // find the position of the trailing '1' bit
                int maskPos = _tzcnt_u64(mask); // vectors loaded in opposite order
                // thats why we dont use leading zeros

                // Compute the length of the current string
                // if we start from 0 the first \n char will be curLine begin
                // then we need this check
                uint64_t curLineLength = (pos >= curLineBegin) ? (pos - curLineBegin + maskPos) : maskPos;

                // Set the beginning of the next line char after
                // the line separator \n
                curLineBegin += curLineLength + 1;

                // check if it is the longest
                longestLine = (longestLine > curLineLength) ? longestLine : curLineLength;

                // shift the mask to check if we have more line
                // separators \n
                ++maskPos;
                // note right shift since loaded reversed
                // shifting 64-bit int by
                // 64 positions is an UB
                if (maskPos > 63)
                    break;
                else
                    mask >>= maskPos;
            }
            // Advance the pointer to the next 32-byte input chunk
            buffer += 64;
        }
        // Calculate the length of the current line
        curLineLength = pos - curLineBegin;
    }
    // process remainder
    for (; pos < strLength; pos++)
    {
        curLineLength = (inputContents[pos] == '\n') ? 0 : curLineLength + 1;
        longestLine = std::max(curLineLength, longestLine);
    }
    return longestLine;
}