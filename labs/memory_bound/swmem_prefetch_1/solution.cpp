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
    if (hash_map->find(val)) {
      if (i + 1 < lookups.size()) [[likely]]
      {
        __builtin_prefetch(&lookups[i + 1], 0, 1);
      }
      result += getSumOfDigits(val);
    }
  }

  return result;
}
