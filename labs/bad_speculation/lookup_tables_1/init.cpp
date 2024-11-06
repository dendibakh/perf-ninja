#include "solution.hpp"
#include <limits>
#include <random>

#ifdef SOLUTION
void init(std::vector<int> &values, std::vector<std::size_t> & table) {
  std::random_device r;
  std::default_random_engine generator(r());
  generator.seed(0);
  int limit = 151;
  int max_value = limit-1;
  // generate random integer in the closed interval [0,150]
  // the chance of selecting the default bucket is ~33%
  std::uniform_int_distribution<int> distribution(0, max_value);
  for (int i = 0; i < NUM_VALUES; i++) {
    values.push_back(distribution(generator));
  }


  table.resize(limit, 0);
  for (int i = 0 ; i < 13; i++)         table[i] = 0;
  for (int i = 13; i < 29; i++)         table[i] = 1;
  for (int i = 29; i < 41; i++)         table[i] = 2;
  for (int i = 41; i < 53; i++)         table[i] = 3;
  for (int i = 53; i < 71; i++)         table[i] = 4;
  for (int i = 71; i < 83; i++)         table[i] = 5;
  for (int i = 83; i < 100; i++)        table[i] = 6;
  for (int i = 100; i < limit; i++) table[i] = DEFAULT_BUCKET;
}

#else
void init(std::vector<int> &values) {
  std::random_device r;
  std::default_random_engine generator(r());
  // generate random integer in the closed interval [0,150]
  // the chance of selecting the default bucket is ~33%
  std::uniform_int_distribution<int> distribution(0, 150);
  for (int i = 0; i < NUM_VALUES; i++) {
    values.push_back(distribution(generator));
  }
}
#endif
