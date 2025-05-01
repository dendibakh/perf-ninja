#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

constexpr size_t prefetch_distance = 16;

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (size_t i = 0; i < lookups.size(); i++) {
    int prefetch_index = std::min(i + prefetch_distance, lookups.size() - 1);
    hash_map->prefetch(lookups[prefetch_index]);
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
