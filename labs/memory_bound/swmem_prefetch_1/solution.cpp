#include "solution.hpp"

#define PREFETCH_DIST 16

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

  for (int i{}; i < lookups.size(); ++i) {
    int val = lookups[i];
    if (i + PREFETCH_DIST < lookups.size()) hash_map->prefetch(lookups[i+PREFETCH_DIST]);
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
