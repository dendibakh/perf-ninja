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

  int val1 = lookups[0], val2 = lookups[1];
  hash_map->prefetch(val1);
  hash_map->prefetch(val2);
  for (int i = 2; i < lookups.size(); i += 2) {
    int new_val1 = lookups[i], new_val2 = lookups[i+1];
    hash_map->prefetch(new_val1);
    hash_map->prefetch(new_val2);
    if (hash_map->find(val1))
      result += getSumOfDigits(val1);
    if (hash_map->find(val2))
      result += getSumOfDigits(val2);
    val1 = new_val1;
    val2 = new_val2;
  }
  if (hash_map->find(val1))
      result += getSumOfDigits(val1);
  if (hash_map->find(val2))
    result += getSumOfDigits(val2);

  return result;
}
