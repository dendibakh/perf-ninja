#include <memory>
#include <vector>

#if defined(__APPLE__) && defined(__MACH__)
  // In Apple processors (such as M1, M2 and later), L2 cache operates on 128B cache lines.
  #define ON_MACOS
  #define CACHELINE_SIZE 128
#else
  #define CACHELINE_SIZE 64
#endif

#if defined(_MSC_VER)
  #define RESTRICT __restrict
#else
  #define RESTRICT __restrict__
#endif

template <typename T>
class CacheLineAlignedAllocator {
public:
  using value_type = T;
  static std::align_val_t constexpr ALIGNMENT{CACHELINE_SIZE};
  [[nodiscard]] T* allocate(std::size_t N) {
    return reinterpret_cast<T*>(::operator new[](N * sizeof(T), ALIGNMENT));
  }
  void deallocate(T* allocPtr, [[maybe_unused]] std::size_t N) {
    ::operator delete[](allocPtr, ALIGNMENT);
  }
};
template<typename T> 
using AlignedVector = std::vector<T, CacheLineAlignedAllocator<T> >;

// ******************************************
// Change this place
// ******************************************
// hint: use AlignedVector instead of std::vector 
//       to align the beginning of a matrix
using Matrix = std::vector<float>;
// ******************************************

static bool isCacheLineAligned(const Matrix& m) {
  uint64_t ptr = (uint64_t)m.data();
  return ptr % CACHELINE_SIZE == 0;
}

int n_columns(int N);
void initRandom(Matrix &matrix, int N, int K);
void initZero(Matrix &matrix, int N, int K);
void copyFromMatrix(const Matrix &from, Matrix &to, int N, int K);
void interchanged_matmul(float* RESTRICT A, 
                         float* RESTRICT B,
                         float* RESTRICT C, int N, int K);
void blocked_matmul     (float* RESTRICT A, 
                         float* RESTRICT B,
                         float* RESTRICT C, int N, int K);