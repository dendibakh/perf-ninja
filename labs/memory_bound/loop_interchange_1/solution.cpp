
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

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  for (int i = 0; i < N; i++) {
    for (int k = 0; k < N; k++) {
      for (int j = 0; j < N; j++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

// Compute integer power of a given square matrix
Matrix power(const Matrix &input, const uint32_t k) {
  std::unique_ptr<Matrix> tempMatrix(new Matrix());

  // Temporary products
  std::unique_ptr<Matrix> productCurrent(new Matrix());

  // Temporary elements = a^(2^integer)
  std::unique_ptr<Matrix> elementCurrent(new Matrix());

  // Initial values
  identity(*productCurrent);
  *elementCurrent = input;

  // Use binary representation of k to be O(log(k))
  for (auto i = k; i > 1; i /= 2) {
    if (i & 1) {
      // Multiply the product by element
      multiply(*tempMatrix, *productCurrent, *elementCurrent);
      std::swap(tempMatrix, productCurrent);
    }

    // Square an element
    multiply(*tempMatrix, *elementCurrent, *elementCurrent);
    std::swap(tempMatrix, elementCurrent);
  }

  multiply(*tempMatrix, *productCurrent, *elementCurrent);

  return std::move(*tempMatrix);
}
