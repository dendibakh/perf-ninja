#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  const int N = 4;
  const auto Batch = size / N;
  for (int ii = 0; ii < N; ii++) {
      for (int jj = 0; jj < N; jj++) {
          int di = Batch * ii;
          int dj = Batch * jj;
          for (int i = di; i < (ii == N - 1 ? size : di + Batch); i++) {
              for (int j = dj; j < (jj == N - 1 ? size : dj + Batch); j++) {
                  out[i][j] = in[j][i];
              }
          }
      }
  }

  return out[0][size - 1];
}
