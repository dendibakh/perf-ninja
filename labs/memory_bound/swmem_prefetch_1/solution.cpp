#include "solution.hpp"
#define SOLUTION
static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

#ifdef SOLUTION
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for(int i =0;i<lookups.size();i++)
  {
    hash_map->prefetch(lookups[i+1]);
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}
#else
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
#endif
