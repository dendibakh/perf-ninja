#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int blockSize = 8;
  for (int i = 0; i < size; i+=blockSize) {
    for (int j = 0; j < size; j+=blockSize) {
      for(int x=i; x<i+blockSize; ++x){
        for(int y=j; y<j+blockSize; ++y){
          out[x][y] = in[y][x];
        }
      }
    }
  }
  return out[0][size - 1];
}
