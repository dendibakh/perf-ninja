#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int tile_size = 32;
  for (int i = 0; i < size; i += tile_size) {
    for (int j = 0; j < size; j += tile_size) {
      int max_ii = std::min(i + tile_size, size);
      int max_jj = std::min(j + tile_size, size);

      for (int jj = j; jj < max_jj; jj++) {
        for (int ii = i; ii < max_ii; ii++) {
          __builtin_prefetch(&in[jj][ii + 1]);
        }
      }

      for (int ii = i; ii < max_ii; ii++) {
        for (int jj = j; jj < max_jj; jj++) {
          out[ii][jj] = in[jj][ii];
        }
      }
    }
  }

  return out[0][size - 1];
}
