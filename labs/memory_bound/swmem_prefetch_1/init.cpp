#include "solution.hpp"
#include <limits>
#include <random>

void init(hash_map_t *hash_map, std::vector<int> &lookups) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(
      0, std::numeric_limits<int>::max());
  int val = distribution(generator);
  hash_map->prefetch(val);
  for (int i = 0; i < HASH_MAP_SIZE; i++) {
    int new_val = distribution(generator);
    hash_map->prefetch(new_val);
    hash_map->insert(val);
    val = new_val;
    
  }

  for (int i = 0; i < NUMBER_OF_LOOKUPS; i++) {
    lookups.push_back(distribution(generator));
  }
}