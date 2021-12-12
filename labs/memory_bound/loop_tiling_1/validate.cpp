#include "solution.hpp"
#include <iostream>

bool original_solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  auto size = in.size();
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      out[i][j] = in[j][i];

  return out[0][size - 1];
}

bool matrices_equal(MatrixOfDoubles &m1, MatrixOfDoubles &m2) {
  if (m1.size() != m2.size())
    return false;

  auto size = m1.size();
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      if (m1[i][j] != m2[i][j])
        return false;

  return true;
}

int main() {
  MatrixOfDoubles in;
  MatrixOfDoubles out;
  MatrixOfDoubles out_golden;

  constexpr int N = 2001;
  in.resize(N, std::vector<double>(N, 0.0));
  out.resize(N, std::vector<double>(N, 0.0));
  out_golden.resize(N, std::vector<double>(N, 0.0));

  // Init benchmark data
  initMatrix(in);

  original_solution(in, out_golden);
  solution(in, out);

  if (!matrices_equal(out, out_golden)) {
    std::cerr << "Validation Failed\n";
    return 1;
  }

  std::cout << "Validation Successful\n";
  return 0;
}
