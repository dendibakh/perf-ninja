#include "solution.hpp"
#include <algorithm>
#include <cmath>

#define SOLUTION

#ifdef SOLUTION
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  // constexpr int KiB = 1024;
  // constexpr int L1_size = 32*KiB;  //
  // constexpr int tile_size = (int)std::floor(std::sqrt(L1_size / sizeof(double)));
  constexpr int tile_size = 64;
  // int lim = (size / tile_size) * tile_size;
  for (int i = 0; i < size; i += tile_size) {
    for (int j = 0; j < size; j += tile_size) {

      for (int ii = i; ii < std::min(i+tile_size, size); ii++) {
        for (int jj = j; jj < std::min(j+tile_size, size); jj++) {
          out[ii][jj] = in[jj][ii];
        }
      }
    }

  }
  // for (int i = lim; i < size; i++) {
  //   for (int j = lim; j < size; j++) {
  //     out[i][j] = in[j][i];
  //   }
  // }

  return out[0][size - 1];
}

#else

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}

#endif
