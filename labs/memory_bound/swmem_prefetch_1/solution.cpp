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

  for (int idx = 0; idx < lookups.size(); idx++) {
    if (hash_map->find(lookups[idx]))
      result += getSumOfDigits(lookups[idx]);

    __builtin_prefetch(&hash_map->m_vector[lookups[idx + 1]]);
  }

  return result;
}
