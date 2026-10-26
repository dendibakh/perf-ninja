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

  for (int i = 0; i < lookups.size(); ++i)
  {
    hash_map->prefetch(lookups[i+1]);
    hash_map->prefetch(lookups[i+2]);
    hash_map->prefetch(lookups[i+3]);
    hash_map->prefetch(lookups[i+4]);
    hash_map->prefetch(lookups[i+5]);
    hash_map->prefetch(lookups[i+6]);
    const int& val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
