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

  for (int i = 0; i < lookups.size(); i++) {
     __builtin_prefetch(hash_map->get_address(lookups[i + 16]));
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}
