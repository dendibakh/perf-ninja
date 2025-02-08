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

  for (size_t i = 0, size = lookups.size(); i < size; ++i) {
    if ((i + 96) < size) {
      __builtin_prefetch(&hash_map->m_vector[lookups[i + 96] % hash_map->N_Buckets]);
    }
    const int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
