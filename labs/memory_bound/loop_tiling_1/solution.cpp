#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  const int tile_size = 32;
  for (int i = 0; i < size; i += tile_size) {
    for (int j = 0; j < size; j += tile_size) {
      for (int r = i; r < std::min(size, i + tile_size); ++r) {
        for (int c = j; c < std::min(size, j + tile_size); ++c) {
          out[r][c] = in[c][r];
        }
      }
    }
  }
  return out[0][size - 1];
}
