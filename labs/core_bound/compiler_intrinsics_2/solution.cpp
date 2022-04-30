#include "solution.hpp"

#include <iostream>


bool ContainsZeroByte(uint64_t v) {
  return ((v - UINT64_C(0x0101010101010101)) & ~v &
          UINT64_C(0x8080808080808080)) != 0;
}

unsigned solution(const std::string& inputContents) {
  const char* beg = inputContents.c_str();
  const char* end = beg + inputContents.length();
  uint64_t u64beg = reinterpret_cast<uint64_t>(beg);
  uint64_t u64beg_aligned =
      u64beg % 8 == 0 ? u64beg : u64beg + (8 - u64beg % 8);
  uint64_t u64end = reinterpret_cast<uint64_t>(end);
  uint64_t u64end_aligned = u64end - u64end % 8;

  const char* beg_aligned = reinterpret_cast<const char*>(u64beg_aligned);
  ptrdiff_t longest = 0;
  const char* last_line_beg = beg;
  for (const char* ch = beg; ch < beg_aligned; ch++) {
    if (*ch == '\n') {
      longest = std::max<ptrdiff_t>(longest, ch - last_line_beg);
      last_line_beg = ch + 1;
    }
  }

  const uint64_t* pu64_beg = reinterpret_cast<const uint64_t*>(u64beg_aligned);
  const uint64_t* pu64_end = reinterpret_cast<const uint64_t*>(u64end_aligned);

  for (const uint64_t* pu64 = pu64_beg; pu64 < pu64_end; pu64++) {
    constexpr uint64_t kMask = UINT64_C(0x0A0A0A0A0A0A0A0A);
    uint64_t u64 = *pu64;
    if (!ContainsZeroByte(u64 ^ kMask)) {
      continue;
    }
    const char* pch = reinterpret_cast<const char*>(pu64);
    for (int i = 0; i < 8; i++) {
      if (pch[i] == '\n') {
        longest = std::max<ptrdiff_t>(longest, &pch[i] - last_line_beg);
        last_line_beg = &pch[i + 1];
      }
    }
  }

  const char* end_aligned = reinterpret_cast<const char*>(u64end_aligned);
  for (const char* ch = end_aligned; ch < end; ch++) {
    if (*ch == '\n') {
      longest = std::max<ptrdiff_t>(longest, ch - last_line_beg);
      last_line_beg = ch + 1;
    }
  }

  longest = std::max<ptrdiff_t>(longest, end - last_line_beg);

  return static_cast<unsigned>(longest);
}
