#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  static constexpr int TILE_SIZE = 16;

  for (int tileCol = 0; tileCol < size; tileCol += TILE_SIZE) {
    for (int tileRow = 0; tileRow < size; tileRow += TILE_SIZE) {
        for (int row = tileRow; row < std::min(tileRow + TILE_SIZE, size); row++) {
          for (int col = tileCol; col < std::min(tileCol + TILE_SIZE, size); col++) {
            out[col][row] = in[row][col];
        }
      }
    }
  }
  return out[0][size - 1];
}
