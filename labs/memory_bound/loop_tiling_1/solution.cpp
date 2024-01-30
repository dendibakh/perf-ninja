#include "solution.hpp"
#include <algorithm>

constexpr int TILE_SIZE = 16;

static void work_tile(const MatrixOfDoubles &in, MatrixOfDoubles &out, const int i_offset, const int j_offset, const int size) {
  for (int i = i_offset; i < std::min(i_offset + TILE_SIZE, size); ++i) {
    for (int j = j_offset; j < std::min(j_offset + TILE_SIZE, size); ++j) {
      out[i][j] = in[j][i];
    }
  }
}

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  const int size = in.size();
  const int tile_count = in.size() / TILE_SIZE + 1;
  
  for (int i = 0; i < tile_count; ++i) {
    for (int j = 0; j < tile_count; ++j) {
      work_tile(in, out, i * TILE_SIZE, j * TILE_SIZE, size);
    }
  }
  return out[0][size - 1];
}
