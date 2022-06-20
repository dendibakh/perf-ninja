#include <cstddef>

// Simplest one-time arena allocator
// T - type of objects.
// N - size in bytes.
// Allocate objects of T consecutively.
template <class T, size_t N> class Arena {
public:
  Arena() : curPtr(arena) {}

  Arena(const Arena &) = delete;
  Arena &operator=(const Arena &) = delete;

  T *allocate() {
    auto endPtr = arena + N;
    if (endPtr - curPtr < sizeof(T))
      return nullptr;

    T *savePtr = (T *)curPtr;
    curPtr += sizeof(T);
    return savePtr;
  }
  // no dealloc method - rely on RAII to dealloc arena

private:
  alignas(128) std::byte arena[N];
  std::byte *curPtr = nullptr;
};
