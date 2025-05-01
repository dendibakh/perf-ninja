#include "solution.hpp"
#include <limits>
#include <random>

void init(hash_map_t *hash_map, std::vector<int> &lookups) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(
      0, std::numeric_limits<int>::max());
  int value = distribution(generator);
  int value_next = distribution(generator);
  for (int i = 0; i < HASH_MAP_SIZE; i++) {
    hash_map->prefetch(value_next);
    hash_map->insert(value);
    value = value_next;
    value_next = distribution(generator);
  }

  for (int i = 0; i < NUMBER_OF_LOOKUPS; i++) {
    lookups.push_back(distribution(generator));
  }
}
