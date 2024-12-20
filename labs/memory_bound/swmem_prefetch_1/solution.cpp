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
    int val = lookups[i];
    if (i  < lookups.size() - 1) {
        int future_val = lookups[i + 1];
        int future_bucket = future_val % hash_map->getNBuckets(); 
        __builtin_prefetch(&((hash_map->getVector())[future_bucket]));
    }
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
