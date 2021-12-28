#include "solution.hpp"
#include <utility>

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

  constexpr int PREFETCH_ELEMENTS = 16;
  const int size = static_cast<int>(lookups.size());

  for (int i = 0; i < std::min(size, PREFETCH_ELEMENTS); ++i) {
    hash_map->prefetchForVal(lookups[i]);
  }

  for (int i = 0; i < size - PREFETCH_ELEMENTS; ++i) {
    const int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }

    hash_map->prefetchForVal(lookups[i + PREFETCH_ELEMENTS]);
  }

  for (int i = std::max(0, size - PREFETCH_ELEMENTS); i < size; ++i) {
    const int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
