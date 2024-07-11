#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int TILE_SIZE = 16;
  for (int i = 0; i < size; i += TILE_SIZE) {
    for (int j = 0; j < size; j += TILE_SIZE) {
      for (int ii = i; ii < std::min(i + TILE_SIZE, size); ++ii) {
        for (int jj = j; jj < std::min(j + TILE_SIZE, size); ++jj) {
          out[ii][jj] = in[jj][ii];
        }
      }
    }
  }

  return out[0][size - 1];
}
