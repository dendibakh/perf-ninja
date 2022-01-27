#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out) {
  static constexpr int TILE_SIZE = 16;
  int size = in.size();
  for (int ii = 0; ii < size; ii+= TILE_SIZE) {
    for (int jj = 0; jj < size; jj+= TILE_SIZE) {
      for (int i = ii; i < std::min(ii + TILE_SIZE, size); i++) {
        for (int j = jj; j < std::min(jj + TILE_SIZE, size); j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
