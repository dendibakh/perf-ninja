#include "solution.hpp"

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

  const size_t lookupsSize = lookups.size();
  static constexpr size_t prefetchStride = 3;

  for (size_t i = 0; i < lookupsSize - prefetchStride; ++i) {
    hash_map->prefetch(lookups[i + prefetchStride]);
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  for (size_t i = lookupsSize - prefetchStride; i < lookupsSize; ++i) {
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
