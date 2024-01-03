#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  static constexpr int TILE_SIZE = 32;
  int size = in.size();
  for (int ii = 0; ii < size; ii += TILE_SIZE) {
    for (int jj = 0; jj < size; jj += TILE_SIZE) {
      for (int i = ii; i < std::min(size, ii + TILE_SIZE); i++) {
        for (int j = jj; j < std::min(size, jj + TILE_SIZE); j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }
  return out[0][size - 1];
}
