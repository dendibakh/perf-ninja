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
  int cnt = 0;
  static constexpr int step = 512;
  for (auto it = lookups.begin(); it != lookups.end(); it++) {
    if (cnt++ % step == 0) {
      hash_map->prefetch(it, it + step, lookups.end());
    }
    bool found = hash_map->find(*it);
    result += getSumOfDigits(*it * found);
  }

  return result;
}
