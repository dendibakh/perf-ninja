#include "solution.hpp"
#include <algorithm>
#include <iostream>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int n = 5;
  for (int ii = 0; ii < size; ii += n)
  {
    for (int jj = 0; jj < size; jj += n)
      for (int i = ii; i < std::min(ii + n, size); i++)
      {
        for (int j = jj; j < std::min(jj + n, size); j++)
        {
          out[i][j] = in[j][i];
        }
      }
  }
  return out[0][size - 1];
}
