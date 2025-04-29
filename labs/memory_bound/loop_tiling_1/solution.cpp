#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int ii = 0; ii < size; ii += i_stride) {
    for (int jj = 0; jj < size; jj += j_stride) {
      for (int i = ii; i < ii + i_stride; i++) {
        for (int j = jj; j < jj + j_stride; j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }
  return out[0][size - 1];
}
