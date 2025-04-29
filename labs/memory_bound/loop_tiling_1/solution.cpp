#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int ii = 0; ii < size; ii += i_stride) {
    int i_limit = std::min(size, ii + i_stride);
    for (int jj = 0; jj < size; jj += j_stride) {
      int j_limit = std::min(size, jj + j_stride);
      for (int i = ii; i < i_limit; i++) {
        for (int j = jj; j < j_limit; j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }
  return out[0][size - 1];
}
