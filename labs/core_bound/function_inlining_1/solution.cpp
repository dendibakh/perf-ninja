
#include "solution.h"
#include <algorithm>
#include <stdlib.h>

#define LIKELY(cond) (__builtin_expect(!!(cond), 1))
#define UNLIKELY(cond) (__builtin_expect(!!(cond), 0))

struct compare {
  inline bool operator() (const S& a, const S& b) {
    return a.key1 < b.key1;
  }
};

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), compare());
}
