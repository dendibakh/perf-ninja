#include "solution.hpp"
#include <algorithm>

#define SOLUTION
static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}


constexpr int lookAhead = 16;
#ifdef SOLUTION
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  for (int i = 0; i < lookups.size() - lookAhead; i++) 
  {
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->prefetch(lookups[i + lookAhead]); // So only do a single prefetch for other instruction
  }

  for (int i = lookups.size() - lookAhead; i < lookups.size(); i++) 
  { // get the already prefetched values
    int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val); 
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
