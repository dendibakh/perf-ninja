
#include "solution.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

static bool equals(const Matrix &a, const Matrix &b) {
  constexpr int maxErrors = 10;
  const float epsilon = std::sqrt(std::numeric_limits<float>::epsilon());

  int errors = 0;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      float va = a[i][j];
      float vb = b[i][j];
      float error = std::abs(va - vb);
      if (error >= epsilon) {
        std::cerr << "Result[" << i << ", " << j << "] = " << va
                  << ". Expected[" << i << ", " << j << "] = " << vb
                  << std::endl;
        if (++errors >= maxErrors)
          return false;
      }
    }
  }
  return 0 == errors;
}

int main() {
  constexpr int k = 15;
  constexpr int k1 = 5;

  std::unique_ptr<Matrix> a(new Matrix());
  std::unique_ptr<Matrix> b(new Matrix());
  std::unique_ptr<Matrix> c(new Matrix());
  std::unique_ptr<Matrix> d(new Matrix());

  init(*a);
  zero(*b);
  identity(*c);
  identity(*d);
  {
    multiply(*b, *a, *d);
    if (!equals(*b, *a)) {
      std::cerr << "Validation Failed. a * 1" << std::endl;
      return 1;
    }
  }
  {
    multiply(*b, *a, *a);
    *c = power(*a, 2);
    if (!equals(*b, *c)) {
      std::cerr << "Validation Failed. a^2" << std::endl;
      return 1;
    }
  }
  *b = power(*a, k);
  *c = power(*a, k1);
  *d = power(*a, k - k1);
  multiply(*a, *c, *d);
  if (!equals(*a, *b)) {
    std::cerr << "Validation Failed. a^k" << std::endl;
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
