#include "solution.hpp"
#include <algorithm>


const int TILE_SIZE = 16;

bool solution1(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int n = size / TILE_SIZE;
  if(size % TILE_SIZE != 0) {
    n++;
  }

  //can be rewritten better
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

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  
  for (int ii = 0; ii < size; ii+=TILE_SIZE) {
    for (int jj = 0; jj < size; jj+=TILE_SIZE) {

      for(int i = ii; i < std::min(ii + TILE_SIZE, size); i++) {
        for(int j = jj; j < std::min(jj + TILE_SIZE, size); j++) {
          out[j][i] = in[i][j];
        }
      }

    }
  }

  return out[0][size - 1];
}


bool base_solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
