#include "solution.hpp"
#include <algorithm>
#include <iostream>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int n = 5;
  for (int ii = 0; ii < size - n; ii += n)
  {
    for (int jj = 0; jj < size - n; jj += n)
      for (int i = ii; i < ii + n; i++)
      {
        for (int j = jj; j < jj + n; j++)
        {
          out[i][j] = in[j][i];
        }
      }
  }
  return out[0][size - 1];
}
