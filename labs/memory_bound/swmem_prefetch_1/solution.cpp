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

  int bucket = 8;

  for (int i = 0; i < lookups.size() - bucket; i++) {
    int val = lookups[i];
    if (hash_map->find(val)) {
      hash_map->pre_fetch_find(lookups[i + bucket]);
      result += getSumOfDigits(val);
    }
  }

  for (int i = lookups.size() - bucket; i < lookups.size(); i++) {
    int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
