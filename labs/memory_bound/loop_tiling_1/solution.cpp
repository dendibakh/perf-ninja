#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  static constexpr int TILE_SIZE = 32;
  int size = in.size();
  for (int i = 0; i < size; i+=TILE_SIZE) {
    for (int j = 0; j < size; j+=TILE_SIZE) {
      for (int k = i; k < std::min(size, i + TILE_SIZE); k++) 
        for (int l = j; l < std::min(size, j + TILE_SIZE); l++)
          out[k][l] = in[l][k];
    }
  }
  return out[0][size - 1];
}
