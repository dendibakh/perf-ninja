#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

constexpr int lookahead = 32; 

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  
  for (int i = 0; i < lookups.size() - lookahead; ++i) {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->prefetch(lookups[i + lookahead]);
  }

  for (int i = lookups.size() - lookahead; i < lookups.size(); ++i) {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
