
#include "solution.h"
#include <memory>
#include <string_view>

// Make zero matrix
void zero(Matrix &result) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      result[i][j] = 0;
    }
  }
}

// Make identity matrix
void identity(Matrix &result) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      result[i][j] = 0;
    }
    result[i][i] = 1;
  }
}

Matrix transpose(const Matrix& A) {
    Matrix B{};

    for (uint16_t i = 0; i < N; ++i) {
        for (uint16_t j = 0; j < N; ++j) {
            B[j][i] = A[i][j];
        }
    }

    return B;
}

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  // Baseline 
  // --------------------------------------------------------------
  // Benchmark                    Time             CPU   Iterations
  // --------------------------------------------------------------
  // bench1/iterations:3  977 988 862 ns    977 529 618 ns       3

  // for (uint16_t i = 0; i < N; i++) {
  //   for (uint16_t j = 0; j < N; j++) {
  //     for (uint16_t k = 0; k < N; k++) {
  //       result[i][j] += a[i][k] * b[k][j];
  //     }
  //   }
  // }

  //Local Variable x3 faster
  // --------------------------------------------------------------
  // Benchmark                    Time             CPU   Iterations
  // --------------------------------------------------------------
  // bench1/iterations:3  328 281 838 ns    327 911 058 ns       3

  // float temp;
  // for (uint16_t i = 0; i < N; i++) {
  //   for (uint16_t j = 0; j < N; j++) {
  //     temp = 0;
  //     for (uint16_t k = 0; k < N; k++) {
  //       temp += a[i][k] * b[k][j];
  //     }
  //     result[i][j] += temp;
  //   }
  // }

  // Loop Interchange
  // --------------------------------------------------------------
  // Benchmark                    Time             CPU   Iterations
  // --------------------------------------------------------------
  // bench1/iterations:3   99 124 107 ns     99 057 236 ns     3
  for (uint16_t i = 0; i < N; i++) {
    for (uint16_t k = 0; k < N; k++) {
      for (uint16_t j = 0; j < N; j++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

// Compute integer power of a given square matrix
Matrix power(const Matrix &input, const uint32_t k) {
  // Temporary products
  std::unique_ptr<Matrix> productCurrent(new Matrix());
  std::unique_ptr<Matrix> productNext(new Matrix());

  // Temporary elements = a^(2^integer)
  std::unique_ptr<Matrix> elementCurrent(new Matrix());
  std::unique_ptr<Matrix> elementNext(new Matrix());

  // Initial values
  identity(*productCurrent);
  *elementCurrent = input;

  // Use binary representation of k to be O(log(k))
  for (auto i = k; i > 0; i /= 2) {
    if (i % 2 != 0) {
      // Multiply the product by element
      multiply(*productNext, *productCurrent, *elementCurrent);
      std::swap(productNext, productCurrent);

      // Exit early to skip next squaring
      if (i == 1)
        break;
    }

    // Square an element
    multiply(*elementNext, *elementCurrent, *elementCurrent);
    std::swap(elementNext, elementCurrent);
  }

  return std::move(*productCurrent);
}
