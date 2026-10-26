#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

// Baseline
// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1           52.3 ms         52.3 ms           13
// -----------------------------------------------------
// 34,664,970,612      cycles                                                                  (39.98%)
// 11,601,460,994      instructions                                                            (49.99%)
//     24,882,149      l2_rqsts.demand_data_rd_hit                                             (50.01%)
//     134,075,131      l2_rqsts.demand_data_rd_miss                                            (50.02%)
//     682,119,510      mem_load_retired.l1_hit                                                 (50.03%)
//     71,367,273      mem_load_retired.l1_miss                                                (50.02%)
//         692,985      mem_load_retired.l2_hit                                                 (40.00%)
//     70,536,771      mem_load_retired.l2_miss                                                (39.99%)
//       5,988,049      mem_load_retired.l3_hit                                                 (39.98%)
//     64,626,943      mem_load_retired.l3_miss                                                (39.98%)

int solution_2(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
  return result;
}

// Improved
// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1           26.5 ms         26.5 ms           28
// -----------------------------------------------------
//  9,515,787,601      cycles                                                                  (39.99%)
// 19,261,417,443      instructions                                                            (49.99%)
//     42,970,061      l2_rqsts.demand_data_rd_hit                                             (49.99%)
//    190,538,501      l2_rqsts.demand_data_rd_miss                                            (50.00%)
//  1,143,773,326      mem_load_retired.l1_hit                                                 (50.00%)
//     91,881,709      mem_load_retired.l1_miss                                                (50.01%)
//      1,036,623      mem_load_retired.l2_hit                                                 (40.01%)
//     91,055,503      mem_load_retired.l2_miss                                                (40.01%)
//      7,770,414      mem_load_retired.l3_hit                                                 (40.00%)
//     83,382,844      mem_load_retired.l3_miss                                                (39.99%)

//   10.572587510 seconds time elapsed

static constexpr auto prefetch_step = 16;
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  const auto size = lookups.size(); 

  if(size <= prefetch_step){
    for (int val : lookups) {
      if (hash_map->find(val))
        result += getSumOfDigits(val);
    }

    return result;
  }

  for (auto i = 0; i + prefetch_step < size; i++) {
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
    hash_map->prefetch_find(lookups[i + prefetch_step]);
  }

  for (auto i = size - prefetch_step; i < size; i++) {
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}

