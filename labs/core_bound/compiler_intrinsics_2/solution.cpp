#include "solution.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>

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
    uint32_t current_chunk = 0;
    const uint32_t mask = 0x0A0A0A0Au;

    for (; index + 4 <= inputContents.size(); index += 4) {
        std::memcpy(&current_chunk, inputContents.data() + index, sizeof(current_chunk));
        uint32_t x = current_chunk ^ mask;
        uint32_t matches = (x - 0x01010101u) & ~x & 0x80808080u;

        if (matches == 0) {
            curLineLength += 4;
            continue;
        }
        const unsigned char* bytes =
            reinterpret_cast<const unsigned char*>(&current_chunk);

        for (int i = 0; i < 4; ++i) {
            if (bytes[i] == '\n') {
                longestLine = std::max(longestLine, curLineLength);
                curLineLength = 0;
            } else {
                ++curLineLength;
            }
        }
    }
    for (; index < inputContents.size(); ++index) {
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