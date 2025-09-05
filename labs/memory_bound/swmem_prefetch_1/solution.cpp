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

  for (int i = 0; i + 1 < lookups.size(); i++) {
    const int val = lookups[i];
    if (hash_map->find(val)) {
      hash_map->prefetch(lookups[i+1]);
      result += getSumOfDigits(val);
    }
  }

  if (lookups.size() > 0) {
    if (hash_map->find(lookups.back())) {
      result += getSumOfDigits(lookups.back());
    }
  }

  return result;
}
