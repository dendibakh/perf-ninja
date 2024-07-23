#include "solution.hpp"

#ifdef __x86_64__
#include <immintrin.h>
#include <xmmintrin.h>
#else
#include <arm_neon.h>
// https://stackoverflow.com/questions/11870910/sse-mm-movemask-epi8-equivalent-method-for-arm-neon
int vmovmaskq_u8(uint8x16_t input) {
    uint16x8_t high_bits = vreinterpretq_u16_u8(vshrq_n_u8(input, 7));

    // Merge the even lanes together with vsra. The '??' bytes are garbage.
    // vsri could also be used, but it is slightly slower on aarch64.
    // 0x??03 ??02 ??00 ??01
    uint32x4_t paired16 = vreinterpretq_u32_u16(
            vsraq_n_u16(high_bits, high_bits, 7));
    // Repeat with wider lanes.
    // 0x??????0B ??????04
    uint64x2_t paired32 = vreinterpretq_u64_u32(
            vsraq_n_u32(paired16, paired16, 14));
    // 0x??????????????4B
    uint8x16_t paired64 = vreinterpretq_u8_u64(
            vsraq_n_u64(paired32, paired32, 28));
    // Extract the low 8 bits from each lane and join.
    // 0x4B
    return vgetq_lane_u8(paired64, 0) | ((int) vgetq_lane_u8(paired64, 8) << 8);
}
#endif

#include <bitset>
#include <cstdint>
#include <iostream>
unsigned solution(const std::string &inputContents) {

    const unsigned char *p = (unsigned char *) inputContents.c_str();
    const int n = inputContents.length();
    int i = 0;


    unsigned len = 0;
    unsigned ans = 0;
    for (; i + 63 < n;) {

#ifdef __x86_64__
        __m256i v1 = _mm256_loadu_si256((__m256i *) &p[i]);
        __m256i v2 = _mm256_loadu_si256((__m256i *) &p[i + 32]);
        __m256i n1 = _mm256_cmpeq_epi8(v1, _mm256_set1_epi8('\n'));
        __m256i n2 = _mm256_cmpeq_epi8(v2, _mm256_set1_epi8('\n'));
        uint64_t m1 = _mm256_movemask_epi8(n1);
        uint64_t m2 = _mm256_movemask_epi8(n2);
        uint64_t msk = m1 | (m2 << 32);

#else
        uint8x16_t v1_low = vld1q_u8(&p[i + 0]);
        uint8x16_t v1_high = vld1q_u8(&p[i + 16]);
        uint8x16_t v2_low = vld1q_u8(&p[i + 32]);
        uint8x16_t v2_high = vld1q_u8(&p[i + 48]);

        uint8x16_t newline_vec = vdupq_n_u8('\n');
        uint8x16_t n1_low = vceqq_u8(v1_low, newline_vec);
        uint8x16_t n1_high = vceqq_u8(v1_high, newline_vec);
        uint8x16_t n2_low = vceqq_u8(v2_low, newline_vec);
        uint8x16_t n2_high = vceqq_u8(v2_high, newline_vec);

        uint64_t m1 = vmovmaskq_u8(n1_low) | vmovmaskq_u8(n1_high) << 16;
        uint64_t m2 = vmovmaskq_u8(n2_low) | vmovmaskq_u8(n2_high) << 16;

        uint64_t msk = m1 | (m2 << 32);
#endif
        if (!msk) {
            len += 64;
            i += 64;
        } else {
            unsigned leading_zeroes = __builtin_ctzll(msk);
            len = leading_zeroes ? len : 0;
            len += leading_zeroes;
            ans = std::max(ans, len);
            // move out the current line we're on
            uint64_t new_msk = msk >> (1 + leading_zeroes);

            if (new_msk) {
                // if there's another line in this chunk, find its end
                unsigned new_leading_zeroes = __builtin_ctzll(new_msk);
                ans = std::max(ans, new_leading_zeroes);
                len = 0;
                // we're in a situation such as msk = 0b10010 and new_msk = 0b100
                //                                          ^ leading_zeroes   ^^ new_leading_zeroes
                //                                      ^^^^^ leading_zeroes + new_leading_zeroes + 2
                i += leading_zeroes + new_leading_zeroes + 2;
            } else {
                // number of zeroes to the left of the singular newline
                unsigned new_leading_zeroes = 64 - (leading_zeroes + 1);
                len = new_leading_zeroes;
                // there's only one newline in this chunk, so we're done processing it
                i += 64;
            }
        }
    }
    for (; i < n; ++i) {
        len++;
        len &= -(p[i] != '\n');
        ans = std::max(ans, len);
    }


    return ans;
}
