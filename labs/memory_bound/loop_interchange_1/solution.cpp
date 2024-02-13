
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

  Matrix transposed;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      transposed[i][j] = b[j][i];
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      auto& cell_result = result[i][j];
      auto* a_ptr = a[i].data();
      auto* transposed_ptr = transposed[j].data();
      for (int k = 0; k < N; k++, a_ptr++, transposed_ptr++) {
        cell_result += *a_ptr * *transposed_ptr;
      }
    }
  }
}

// Compute integer power of a given square matrix
Matrix power(const Matrix &input, const uint32_t k) {
  // Temporary products
  Matrix productCurrent;
  Matrix productNext;
  Matrix elementCurrent;
  Matrix elementNext;

  // Initial values
  identity(productCurrent);
  elementCurrent = input;

  // Use binary representation of k to be O(log(k))
  for (auto i = k; i > 0; i /= 2) {
    if (i % 2 != 0) {
      // Multiply the product by element
      multiply(productNext, productCurrent, elementCurrent);
      std::swap(productNext, productCurrent);

      // Exit early to skip next squaring
      if (i == 1)
        break;
    }

    // Square an element
    multiply(elementNext, elementCurrent, elementCurrent);
    std::swap(elementNext, elementCurrent);
  }

  return std::move(productCurrent);
}
