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
  auto size = lookups.size();
  int lookAhead = 8;
  std::size_t nBuckets = hash_map->get_nBuckets();
  std::vector<int> v = hash_map->get_vector();

  for (int i=0; i<size-lookAhead; ++i) {
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);

    //hash_map->prefetch(lookups[i+lookAhead]);
    __builtin_prefetch(&v[(lookups[i+lookAhead])%nBuckets]);
  }

  for (int i=size-lookAhead; i<size; ++i) {
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}