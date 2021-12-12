#include "solution.hpp"

void initMatrix(MatrixOfDoubles &m) {
  auto size = m.size();
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      m[i][j] = (i + j) % 1024;
}
