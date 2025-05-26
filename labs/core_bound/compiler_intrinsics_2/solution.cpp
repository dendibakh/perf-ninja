#include "solution.hpp"
#include <iostream>
#include <cstring>
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

#define SOLUTION

#ifdef SOLUTION
unsigned solution(const std::string &inputContents) {
  const char* data  = inputContents.data();
  const char* const end = data + inputContents.size();
  unsigned longest = 0;

  while (data < end) {
    const void* p = std::memchr(data, '\n', end - data);
    const char* nl = static_cast<const char*>(p);

    unsigned len = (nl ? nl : end) - data;
    longest = std::max(len, longest);

    if (!nl) break;
    data = nl + 1;
  }
  return longest;
}

#else

unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;

  for (auto s : inputContents) {
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
#endif