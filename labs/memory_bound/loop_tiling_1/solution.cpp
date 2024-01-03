#include "solution.hpp"
#include <algorithm>

const int TILE_SIZE = 128;

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int k = 0; k < size; k += TILE_SIZE) {
    for (int i = 0; i < size; i++) {
      for (int j = k; j < size && j < k + TILE_SIZE; j++) {
        out[i][j] = in[j][i];
      }
    }
  }
  return out[0][size - 1];
}
