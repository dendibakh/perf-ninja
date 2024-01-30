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
  constexpr int prefetch_ahead = 16;
  for (int i = 0; i < lookups.size() - prefetch_ahead; ++i) {
    if (hash_map->find(lookups[i])) {
      result += getSumOfDigits(lookups[i]);
    }
    hash_map->prefetch(lookups[i+prefetch_ahead]);
  }
  for (int i = lookups.size() - prefetch_ahead; i < lookups.size(); ++i) {
    if (hash_map->find(lookups[i])) {
      result += getSumOfDigits(lookups[i]);
    }
  }

  return result;
}
