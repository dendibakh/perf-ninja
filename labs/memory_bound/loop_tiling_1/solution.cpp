#include "solution.hpp"
#include <algorithm>

#ifdef _WIN32
    const int TILE_SIZE = 12;
#else
    const int TILE_SIZE = 8;
#endif
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int n = size / TILE_SIZE;
  if(size % TILE_SIZE != 0) {
    n++;
  }

  for(int tile_i = 0; tile_i < n; tile_i++) {
    for(int tile_j = 0; tile_j < n; tile_j++) {
        for(int i = tile_i*TILE_SIZE; i<(tile_i+1)*TILE_SIZE && i < size; i++) {
            for(int j = tile_j * TILE_SIZE; j < (tile_j + 1) * TILE_SIZE && j < size; j++) {
                int m = tile_j * TILE_SIZE + j%TILE_SIZE;
                int n = tile_i * TILE_SIZE + i%TILE_SIZE;
                out[m][n] = in[i][j];
            }
        }
    }
  }
  return out[0][size - 1];
}


bool solution1(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
