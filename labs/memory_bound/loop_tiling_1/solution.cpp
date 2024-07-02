#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  constexpr int TILE_SIZE = 16;
  int size = in.size();
  for (int i = 0; i < size; i += TILE_SIZE) {
    for (int j = 0; j < size; j += TILE_SIZE) {
      for (int ii = i; ii < std::min(size, i + TILE_SIZE); ii++) {
        for (int jj = j; jj < std::min(size, j + TILE_SIZE); jj++) {
          out[ii][jj] = in[jj][ii];
        }
      }
    }
  }
  return out[0][size - 1];
}
