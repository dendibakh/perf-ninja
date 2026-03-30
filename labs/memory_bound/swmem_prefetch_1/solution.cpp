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
  int EARLY_PREFETCH_DISTANCE = 16;
  int i = 0;
  int size = lookups.size();
  for (; i < size - EARLY_PREFETCH_DISTANCE; i++) {
    hash_map->prefetch(lookups[i + EARLY_PREFETCH_DISTANCE]);
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  for (; i < size; i++) {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
