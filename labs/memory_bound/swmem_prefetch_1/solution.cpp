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
  static constexpr int step = 16;
  hash_map->prefetch(lookups.begin(), lookups.begin() + step, lookups.end());
  for (auto it = lookups.begin(); it != lookups.end(); it++) {
    hash_map->prefetch(it + step - 1, it + step, lookups.end());
    bool found = hash_map->find(*it);
    result += getSumOfDigits(*it * found);
  }

  return result;
}
