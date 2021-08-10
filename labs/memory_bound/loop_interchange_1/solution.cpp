
#include "solution.h"
#include <memory>

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
    for (int j = 0; j < N; j++) {
      for (int k = 0; k < N; k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

constexpr Matrix identity() {
  Matrix result{};
  for (int i = 0; i < N; i++) {
    result[i][i] = 1;
  }
  return result;
}

Matrix multiply(const Matrix &a, const Matrix &b) {
  Matrix result{};
  for (int i = 0; i < N; i++) {
    for (int k = 0; k < N; k++) {
      for (int j = 0; j < N; j++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }

  return result;
}

// Compute integer power of a given square matrix
Matrix power(const Matrix &input, const uint32_t k) {
  // Temporary products
  auto productCurrent = identity();

  // Temporary elements = a^(2^integer)
  auto elementCurrent = input;

  // Use binary representation of k to be O(log(k))
  for (auto i = k; i > 0; i /= 2) {
    // k is always 2021, it has only 2 zero bits, we can add [[likely]]?
    if (i % 2 != 0) {
      // Multiply the product by element
      productCurrent = multiply(productCurrent, elementCurrent);

      // Exit early to skip next squaring
      if (i == 1)
      {
        break;
      }
    }

    // Square an element
    elementCurrent = multiply(elementCurrent, elementCurrent);
  }

  return productCurrent;
}
