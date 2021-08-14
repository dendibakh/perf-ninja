
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

static Row get_column(const Matrix &m, int column_index) {
  // convert entries stored column-wise into a
  // contiguous array of values to
  // allow vectorization
  Row column_values;
  for (int k = 0; k < N; k++) {
    column_values[k] = m[k][column_index];
  }

  return column_values;
}

static float calc_scalar(const Row &a, const Row &b) {
  float value = 0.f;

  for (int k = 0; k < N; k++) {
    value += a[k] * b[k];
  }

  return value;
}

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  // omit zeroing target matrix as it will be overwritten with the final value
  // zero(result);

  for (int j = 0; j < N; j++) {
    const Row column_values = get_column(b, j);

    for (int i = 0; i < N; i++) {
      // prevent store to memory on every iteration, this potentially allows a
      // more efficient calculation

      const Row &row_values = a[i];
      result[i][j] = calc_scalar(row_values, column_values);
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

  if (k % 2 != 0) {
    // Multiply the product by element
    multiply(*productNext, *productCurrent, *elementCurrent);
    std::swap(productNext, productCurrent);

    // Exit early to skip next squaring
    if (k == 1)
      return *productCurrent;
  }

  // Use binary representation of k to be O(log(k))
  for (auto i = k & ~1u; i > 0; i /= 2) {
    // Square an element
    multiply(*elementNext, *elementCurrent, *elementCurrent);
    std::swap(elementNext, elementCurrent);
  }

  // remove std::move to allow for copy elision (even though this is more of a
  // rounding error)
  return *productCurrent;
}
