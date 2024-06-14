
#include "solution.h"
#include <cstdlib>
#include <cstring>
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

typedef float vec __attribute__((vector_size(64)));

vec *alloc(int n) {
  vec *ptr = (vec *)std::aligned_alloc(64, 64 * n);
  std::memset(ptr, 0, 64 * n);
  return ptr;
}

int nB = (N + 15) / 16;
vec *va = alloc(N * nB);
vec *vb = alloc(N * nB);

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      va[i * nB + j / 16][j % 16] = a[i][j];
      vb[i * nB + j / 16][j % 16] = b[j][i];
    }
  }

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      vec s{};
      for (int k = 0; k < nB; ++k) {
        s += va[i * nB + k] * vb[j * nB + k];
      }
      for (int k = 0; k < 16; k++) {
        result[i][j] += s[k];
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
