#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int tile_size = 4;

  for (int ii = 0; ii < size; ii+=tile_size){
    for (int jj = 0; jj < size; jj+=tile_size) {
      for (int i = ii; i < ii + tile_size; i++) {
        for (int j = jj; j < jj + tile_size; j++) {
          out[i][j] = in[j][i];
        }
    }
    }
  }
  return out[0][size - 1];
}
