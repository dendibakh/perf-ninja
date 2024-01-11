#include "solution.hpp"
#include <algorithm>


bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int blockSize = 64;
  for (int i = 0; i < size; i+=blockSize) {
    for (int j = 0; j < size; j+=blockSize) {
      for(int x=i; x<std::min(i+blockSize, size); ++x){
        for(int y=j; y<std::min(j+blockSize, size); ++y){
          out[x][y] = in[y][x];
        }
      }
    }
  }
  return out[0][size - 1];
}

