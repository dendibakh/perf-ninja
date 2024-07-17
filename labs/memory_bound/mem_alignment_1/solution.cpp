
#include "solution.h"
#include <random>

// ******************************************
// Change this function
// ******************************************
// This function allows you to change the number of columns in a matrix. 
// In other words, it defines how many elements are in each row.
// hint: you need to allocate dummy columns to achieve proper data alignment.
int n_columns(int N) {  
  return N;
}
// ******************************************

// DO NOT change any of the functions below.
// The following applies to all functions below:
// You will notice the functions have `K` argument in addition to `N`.
// This is because these functions are prepared for matrices with aligned rows.
// I.e. a matrix has N x N elements, but its actual dimensions are N x K since
// it has padding. However, only N x N elements are used.
void initRandom(Matrix &matrix, int N, int K) {
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(-0.95f, 0.95f);
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      matrix[i * K + j] = distribution(generator);
}

void initZero(Matrix &matrix, int N, int K) {
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      matrix[i * K + j] = 0.0f;
}

void copyFromMatrix(const Matrix &from, Matrix &to, int N, int K) {
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      to[i * K + j] = from[i * N + j];
}

// A simple GEMM. Use only for small matrices (up to 100 x 100)
void interchanged_matmul(float* RESTRICT A, 
                         float* RESTRICT B,
                         float* RESTRICT C, int N, int K) {
  for (int i = 0; i < N; ++i)
    for (int k = 0; k < N; ++k)
      for (int j = 0; j < N; ++j)
        C[i * K + j] += A[i * K + k] * B[k * K + j];
}

// Here is a blocked version for larger matrix sizes (e.g. 512 x 512 and beyond).
void blocked_matmul(float* RESTRICT A, 
                    float* RESTRICT B,
                    float* RESTRICT C, int N, int K) {
  constexpr int blockSize = 64;
  for (int ii = 0; ii < N; ii += blockSize)
    for (int kk = 0; kk < N; kk += blockSize)
      for (int jj = 0; jj < N; jj += blockSize)
        for (int i = ii; i < std::min(ii + blockSize, N); ++i)
          for (int k = kk; k < std::min(kk + blockSize, N); ++k)
            for (int j = jj; j < std::min(jj + blockSize, N); ++j)                        
              C[i * K + j] += A[i * K + k] * B[k * K + j];
}
