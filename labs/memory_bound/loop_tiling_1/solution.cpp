#include "solution.hpp"
#include <algorithm>
#include <iostream>

// In Intel Advisor we see:
//          Per Loop  Per Inst.   Per Iter.   
// L1, GB   	0.48	  4.80e-05	  2.40e-08	-
// L2, GB 	  2.10	  2.10e-04	  1.05e-07	-
// L3, GB 	  0.96	  9.60e-05	  4.80e-08	-
// DRAM, GB 	0.49	  4.89e-05	  2.45e-08	-

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1           15.0 ms         15.0 ms           44

//  Performance counter stats for './lab':

//      4,152,842,851      cycles                                                                  (39.78%)
//      1,821,749,666      instructions                                                            (49.82%)
//        188,771,948      l2_rqsts.demand_data_rd_hit                                             (49.92%)
//        160,719,330      l2_rqsts.demand_data_rd_miss                                            (50.02%)
//        184,832,533      mem_load_retired.l1_hit                                                 (50.14%)
//        239,929,631      mem_load_retired.l1_miss                                                (50.18%)
//         96,769,120      mem_load_retired.l2_hit                                                 (40.14%)
//        142,399,784      mem_load_retired.l2_miss                                                (40.05%)
//        128,258,894      mem_load_retired.l3_hit                                                 (39.94%)
//         15,032,312      mem_load_retired.l3_miss                                                (39.82%)

//        0.907195101 seconds time elapsed

//        0.856130000 seconds user
//        0.051007000 seconds sys

// Baseline
// bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
//   int size = in.size();

//   for (int i = 0; i < size; i++) {
//     for (int j = 0; j < size; j++) {
//       out[i][j] = in[j][i];
//     }
//   }
//   return out[0][size - 1];
// }

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();

  int TILE_SIZE = 15;

  for(int ii = 0; ii < size; ii += TILE_SIZE) {
    for(int jj = 0; jj < size; jj += TILE_SIZE) {
      for (int i = ii; i < std::min(size, ii + TILE_SIZE); i++) {
        for (int j = jj; j < std::min(size, jj + TILE_SIZE); j++) {
          // std::cout << "[ii,jj]:[i,j] [" << ii << "," << jj << "]" << ":[" << i << "," << j << "]" << std::endl;
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}

bool solution_fix(MatrixOfDoubles &in, MatrixOfDoubles &out, int TILE_SIZE) {
  int size = in.size();

  for(int ii = 0; ii < size; ii += TILE_SIZE) {
    for(int jj = 0; jj < size; jj += TILE_SIZE) {
      for (int i = ii; i < std::min(size, ii + TILE_SIZE); i++) {
        for (int j = jj; j < std::min(size, jj + TILE_SIZE); j++) {
          // std::cout << "[ii,jj]:[i,j] [" << ii << "," << jj << "]" << ":[" << i << "," << j << "]" << std::endl;
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
