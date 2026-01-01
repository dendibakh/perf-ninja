#include "solution.hpp"

#include <cstddef>

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

  constexpr size_t PREFETCH_STEP = 15U;
  size_t i = 0U;
  for (size_t p = PREFETCH_STEP; p < lookups.size(); ++i, ++p) {
    hash_map->prefetch(lookups[p]);

    int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  for ( ; i < lookups.size(); ++i) {
    int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
