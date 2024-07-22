#include "solution.hpp"

#include <cstring>

#ifdef __x86_64__
#include <immintrin.h>
#include <xmmintrin.h>
#endif

#include <iostream>

unsigned solution(const std::string &inputContents) {

    const char *p = inputContents.c_str();
    const int n = inputContents.length();
    int i = 0;


    unsigned len = 0;
    unsigned ans = 0;
    for (; i + 31 < n;) {

#ifdef __x86_64__
        __m256i vals = _mm256_loadu_si256((__m256i *) &p[i]);
        __m256i is_newline = _mm256_cmpeq_epi8(vals, _mm256_set1_epi8('\n'));
        unsigned msk = _mm256_movemask_epi8(is_newline);
#else
        char vals[32];
        memcpy(vals, i + p, 32);

        unsigned msk = 0;
        for (int j = 0; j < 32; ++j)
            msk |= (vals[j] == '\n') << j;
#endif
        if (msk == 0) {
            len += 32;
            i += 32;
        } else {
            const unsigned leading_zeroes = __builtin_ctz(msk);
            len &= -(leading_zeroes != 0);
            ans = std::max(ans, len + leading_zeroes);
            len = 0;
            i += 1 + leading_zeroes;
        }
    }
    for (; i < n; ++i) {
        len++;
        len &= -(p[i] != '\n');
        ans = std::max(ans, len);
    }


    return ans;
}
