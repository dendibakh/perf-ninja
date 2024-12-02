#include "solution.hpp"
#include <algorithm>
#include <cstdio>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int count = 0;
  int chunkSize = 32;
  int chunkCount = size/chunkSize;
  int stride = size/chunkCount;

  for(int x = 0; x < size; x+=stride)
  {
    for (int i = 0; i < size; i++) 
    {
      for (int j = x; j < std::min(size, x+stride); j++) 
      {
        out[i][j] = in[j][i];
      }
    }
  }

  return out[0][size - 1];
}
