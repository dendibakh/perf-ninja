
#include "solution.h"

// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  for (const auto& item : input) {
    const bool cond = (lower <= item.first) && (item.first <= upper);
    // can't get gcc to generate a damn CMOV on its own...
    asm volatile("test    %[cond], %[cond]\n\t"

                 "mov     %[r0Default], %%eax\n\t"
                 "cmovne  %[r0Cond], %%eax\n\t"
                 "mov     %%eax, %[pFirst]\n\t"

                 "mov     %[r1Default], %%eax\n\t"
                 "cmovne  %[r1Cond], %%eax\n\t"
                 "mov     %%eax, %[pSecond]\n\t"

                 "mov     %[cDefault], %[cDest]\n\t"
                 "cmovne  %[cCond], %[cDest]\n\t"

                : [pFirst]     "=r" (output[count].first),
                  [pSecond]    "=r" (output[count].second),
                  [cDest]      "=r" (count)
                : [cond]       "r"  (cond),
                  [r0Default]  "r"  (output[count].first),
                  [r1Default]  "r"  (output[count].second),
                  [r0Cond]     "r"  (item.first),
                  [r1Cond]     "r"  (item.second),
                  [cDefault]   "r"  (count),
                  [cCond]      "r"  (count+1)
                : "%eax");
  }
  return count;
}
