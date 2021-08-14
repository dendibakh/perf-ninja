
#include "solution.h"
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

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  // omit zeroing target matrix as it will be overwritten with the final value
  // zero(result);
  Row column_values;

  for (int j = 0; j < N; j++) {
    // convert entries stored column-wise into a contiguous array of values to
    // allow vectorization
    for (int k = 0; k < N; k++) {
      column_values[k] = b[k][j];
    }

    for (int i = 0; i < N; i++) {
      // prevent store to memory on every iteration, this potentially allows a
      // more efficient calculation

      const Row &row_values = a[i];
      float value = 0.f;
      for (int k = 0; k < N; k++) {
        value += row_values[k] * column_values[k];
      }
      result[i][j] = value;
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

  // remove std::move to allow for copy elision (even though this is more of a
  // rounding error)
  return *productCurrent;
}
