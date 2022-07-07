#include "solution.hpp"
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
unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;
  for (auto s : inputContents) {
    longestLine = std::max(curLineLength, longestLine);
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
  }

  // if no end-of-line in the end
  longestLine = std::max(curLineLength, longestLine);
  return longestLine;
}
