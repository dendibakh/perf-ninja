#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int block_size = 32;
  for (int ii = 0; ii < size; ii += block_size) {
    for (int jj = 0; jj < size; jj += block_size) {
      for (int i = ii; i < std::min(ii + block_size, size); i++) {
        for (int j = jj; j < std::min(jj + block_size, size); j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
