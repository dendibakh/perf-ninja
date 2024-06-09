#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int block_size_l1 = 16;
  constexpr int block_size_l2 = 64;
  constexpr int block_size_l3 = 256;

  for (int iiii = 0; iiii < size; iiii += block_size_l3) {
    for (int jjjj = 0; jjjj < size; jjjj += block_size_l3) {
      for (int i = iiii; i < std::min(iiii + block_size_l3, size);
           i += block_size_l2) {
        for (int j = jjjj; j < std::min(jjjj + block_size_l3, size);
             j += block_size_l2) {
          for (int ii = i; ii < std::min(i + block_size_l2, size);
               ii += block_size_l1) {
            for (int jj = j; jj < std::min(j + block_size_l2, size);
                 jj += block_size_l1) {
              for (int i = ii; i < std::min(ii + block_size_l1, size); i++) {
                for (int j = jj; j < std::min(jj + block_size_l1, size); j++) {
                  out[i][j] = in[j][i];
                }
              }
            }
          }
        }
      }
    }
  }

  // for (int ii = 0; ii < size; ii += block_size) {
  //   for (int jj = 0; jj < size; jj += block_size) {
  //     for (int i = ii; i < std::min(ii + block_size, size); i++) {
  //       for (int j = jj; j < std::min(jj + block_size, size); j++) {
  //         out[i][j] = in[j][i];
  //       }
  //     }
  //   }
  // }

  return out[0][size - 1];
}
