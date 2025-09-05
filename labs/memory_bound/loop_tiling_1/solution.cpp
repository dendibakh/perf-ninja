#include "solution.hpp"
#include <algorithm>

constexpr size_t TILE_WIDTH = 16;

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  size_t size = in.size();
  for (size_t h = 0; h < size; h += TILE_WIDTH) {
    const size_t hLimit = std::min(TILE_WIDTH, size - h);
    for (size_t w = 0; w < size; w += TILE_WIDTH) {
      const size_t wLimit = std::min(TILE_WIDTH, size - w);
      for (size_t i = 0; i < hLimit; i++) {
        for (size_t j = 0; j < wLimit; j++) {
          out[h + i][w + j] = in[w + j][h + i];
        }
      }
    }
  }
  return out[0][size - 1];
}
