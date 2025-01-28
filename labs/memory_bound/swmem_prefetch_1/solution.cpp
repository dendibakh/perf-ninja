#include "solution.hpp"

#ifdef _MSC_VER
#include <xmmintrin.h>
#endif

void prefetch_memory(const void* ptr) {
#ifdef _MSC_VER
    _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#else
    __builtin_prefetch(ptr);
#endif
}

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  constexpr int look_ahead = 16;
  const int sz = lookups.size();
  const int limit = std::max(0, sz - look_ahead);
  for (int i = 0; i < limit; ++i) {
    const int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->prefetch(lookups[i + look_ahead]);
  }
  for (int i = limit; i < sz; ++i) {
    const int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
  return result;
}
