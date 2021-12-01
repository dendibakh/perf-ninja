#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

constexpr int look_ahead = 16;

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (int i = 0; i < lookups.size() - look_ahead; i++) {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->prefetchForVal(lookups[i + look_ahead]);
  }

  for (int i = lookups.size() - look_ahead; i < lookups.size(); i++) {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
