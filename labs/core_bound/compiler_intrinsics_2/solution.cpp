#include "solution.hpp"

#include <cstdint>
#include <iostream>

template <typename Uint>
constexpr Uint PopulateByte(uint8_t b) {
  Uint result = b;
  for (size_t i = 1; i < sizeof(Uint); i *= 2) {
    result = result | (result << i * 8);
  }
  return result;
}

constexpr uint64_t PopCount(uint64_t x) {
  x = x - ((x >> 1) & PopulateByte<uint64_t>(0x55));
  x = (x & PopulateByte<uint64_t>(0x33)) +
      ((x >> 2) & PopulateByte<uint64_t>(0x33));
  x = (x + (x >> 4)) & PopulateByte<uint64_t>(0x0F);
  return x * PopulateByte<uint64_t>(0x01) >> 56;
}

constexpr bool ContainsZeroByte(uint64_t v) {
  return ((v - PopulateByte<uint64_t>(0x01)) & ~v &
          PopulateByte<uint64_t>(0x80)) != 0;
}

constexpr uint64_t Nlz(uint64_t x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  x = x | (x >> 32);

  return PopCount(~x);
}

constexpr uint64_t Ntz(uint64_t x) {
  return PopCount(~x & (x - 1));
}

constexpr uint64_t MarkZeroBytes(uint64_t x) {
  uint64_t y =
      ((x & PopulateByte<uint64_t>(0x7F)) + PopulateByte<uint64_t>(0x7F)) &
      PopulateByte<uint64_t>(0x80);
  return ~(y | x | PopulateByte<uint64_t>(0x7F));
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
    const uint64_t u64 = *pu64;
    const uint64_t masked = u64 ^ kMask;
    if (!ContainsZeroByte(masked)) {
      continue;
    }
    const uint64_t x80ed = MarkZeroBytes(masked);
    const char* pch = reinterpret_cast<const char*>(pu64);
    ptrdiff_t lsf = pch - last_line_beg;
    if (lsf + 8 >= longest) {
      longest = std::max<ptrdiff_t>(longest, lsf + Ntz(x80ed) / 8);
    }

    last_line_beg = pch + 8 - Nlz(x80ed) / 8;
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
