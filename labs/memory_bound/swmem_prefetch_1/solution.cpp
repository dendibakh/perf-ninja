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
  int look_ahead = 2;
  auto size = lookups.size();

  for (int i=0; i<size-look_ahead; ++i) {

    if (hash_map->find(lookups[i])){
      result += getSumOfDigits(lookups[i]);
    }
    hash_map->prefetch(lookups[i+look_ahead]);
      
  }

  for(int i=size-look_ahead; i<size; ++i){
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}