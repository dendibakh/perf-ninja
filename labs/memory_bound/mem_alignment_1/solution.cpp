
#include "solution.h"
#include <random>
#include <cstdio>

// ******************************************
// Change this function
// ******************************************
// This function allows you to change the number of columns in a matrix. 
// In other words, it defines how many elements are in each row.
// hint: you need to allocate dummy columns to achieve proper data alignment.

int n_columns(int N) {
  
  const int size = N * sizeof(float);
  const int N_old = N;

// #define SOLUTION
/*
bench1/_63         34.9 us         34.9 us        20063
bench1/_64         15.6 us         15.6 us        49786
bench1/_65         17.5 us         17.5 us        39933
bench1/_128         127 us          127 us         5666
bench1/_256        1202 us         1200 us          580
bench1/_511       11378 us        11333 us           61
bench1/_512        9854 us         9839 us           71
bench1/_513       10881 us        10859 us           65
bench1/_1024      89534 us        89528 us            6
*/

#define SOLUTION_1
/*
CacheLine = 64

bench1/_63         35.4 us         35.4 us        19992
bench1/_64         12.1 us         12.1 us        57606
bench1/_65         13.4 us         13.4 us        52066
bench1/_128        93.1 us         92.9 us         7329
bench1/_256         749 us          747 us          957
bench1/_511       10799 us        10791 us           61
bench1/_512        6202 us         6201 us          113
bench1/_513        8486 us         8486 us           80
bench1/_1024      66050 us        65952 us            9

CacheLine = 128

bench1/_63         35.3 us         35.3 us        20538
bench1/_64         15.7 us         15.7 us        44524
bench1/_65         17.8 us         17.8 us        39351
bench1/_128         126 us          126 us         5525
bench1/_256        1012 us         1012 us          687
bench1/_511       11870 us        11869 us           58
bench1/_512        8124 us         8123 us           86
bench1/_513        8643 us         8642 us           80
bench1/_1024      67572 us        67567 us            8
*/

#ifdef SOLUTION // align size by power-of-two boundary
  int aligned_size = 1;
  int tmp_size = size;

  while (tmp_size > 0) {
    tmp_size = tmp_size >> 1;
    aligned_size = aligned_size << 1;
  }

  aligned_size = aligned_size % size == 0 ? size : aligned_size;
  N = aligned_size / sizeof(float);
#endif

#ifdef SOLUTION_1 // align by cache line size
  constexpr int CacheLineSize = 64;
  int aligned_size = ((size / CacheLineSize) + 1) * CacheLineSize;
  N = aligned_size / sizeof(float);
#endif

  // printf("N_old = %d, N = %d\n", N_old, N);

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
