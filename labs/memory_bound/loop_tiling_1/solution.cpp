#include "solution.hpp"
#include <algorithm>

static constexpr int TILE = 8;
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int nsize = (size - TILE) / TILE * TILE;
  for (int a = 0; a < nsize; a += TILE) {
    for (int b = 0; b < nsize; b += TILE) {
      for (int i = 0; i < TILE; i++) {
        for (int j = 0; j < TILE; j++) {
          out[a + i][b + j] = in[b + j][a + i];
        }
      }
    }
  }
  for (int i = nsize; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      out[i][j] = in[j][i];
    }
  }
  for (int i = 0; i < size; ++i) {
    for (int j = nsize; j < size; ++j) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
