
#include "solution.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
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

constexpr int kH = 6, kW = 16;
constexpr int kVecSize = 8;

typedef float vec __attribute__((vector_size(kVecSize * 4)));

// update kH*kW matrix C[x:x+kH][y:y+kW]
// from A [x:x+kH][l:r] and B[l:r][y:y+kW]
void kernel(float *a, vec *b, vec *c, int x, int y, int l, int r, int n) {
  assert(kW % kVecSize == 0);
  vec t[kH][kW / kVecSize]{};

  // c[i] = a[i][k] * b[k][0] concat with a[i][j] * b[k][1]
  for (int k = l; k < r; k++) {
    for (int i = 0; i < kH; i++) {
      // broadcast a[i][k]
      vec bc = vec{} + a[(x + i) * n + k];
      for (int j = 0; j < kW / kVecSize; j++) {
        t[i][j] += bc * b[(k * n + y) / kVecSize + j];
      }
    }
  }

  for (int i = 0; i < kH; i++) {
    for (int j = 0; j < kW / kVecSize; j++) {
      c[((x + i) * n + y) / kVecSize + j] += t[i][j];
    }
  }
}

vec *alloc(int n) {
  vec *ptr = (vec *)std::aligned_alloc(32, 32 * n);
  std::memset(ptr, 0, 32 * n);
  return ptr;
}

float *alloc_float(int n) {
  assert(n % 8 == 0);
  return (float *)std::aligned_alloc(32, n * 4);
}

int nx = (N + kH - 1) / kH * kH;
int ny = (N + kW - 1) / kW * kW;

float *A = alloc_float(nx * ny);
float *B = alloc_float(nx * ny);
float *C = alloc_float(nx * ny);

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {

  memset(C, 0, nx * ny * 4);

  for (int i = 0; i < N; i++) {
    memcpy(&A[i * ny], &a[i], 4 * N);
    memcpy(&B[i * ny], &b[i], 4 * N);
  }

  // for (int i = 0; i < nx; i += kH) {
  //   for (int j = 0; j < ny; j += kW) {
  //     kernel(A, (vec*)B, (vec*)C, i, j, 0, N, ny);
  //   }
  // }

  // cache blocking
  constexpr int s3 = 64;
  constexpr int s2 = 120;
  constexpr int s1 = 240;

  for (int i3 = 0; i3 < ny; i3 += s3) {   // work with B[...][i3...i3+s3]
    for (int i2 = 0; i2 < nx; i2 += s2) { // work with A[i2...i2+s2][...]

      // calculate answer for C[i2...i2+s2][i3...i3+s3]
      for (int i1 = 0; i1 < ny;
           i1 +=
           s1) { // instead of a scalar k, we work with a range (i1 -> i1 + s1)
        // A[i2...i2+s2][i1...i1+s1] and B[i1...i1+s1][i3...i3+s3] // fit in L1
        // / L2 cache use kernel to calculate
        for (int i = i2; i < std::min(i2 + s2, nx); i += kH) {
          for (int j = i3; j < std::min(i3 + s3, ny); j += kW) {
            kernel(A, (vec *)B, (vec *)C, i, j, i1, std::min(i1 + s1, N), ny);
          }
        }
      }
    }
  }

  for (int i = 0; i < N; i++) {
    memcpy(&result[i], &C[i * ny], 4 * N);
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
