#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int TILE_SIZE = 16;
  for (int i = 0; i < size; i+=TILE_SIZE) {
    for (int j = 0; j < size; j+=TILE_SIZE) {
      int bi = std::min(i + TILE_SIZE, size);
      int bj = std::min(j + TILE_SIZE, size);
      for (int ii = i; ii < bi; ii++){
        for (int jj = j; jj < bj; jj++){
          out[ii][jj] = in[jj][ii];
        }
      }
    }
  }
  return out[0][size - 1];
}
