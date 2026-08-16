#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

template<class T>
struct alignas(8 * sizeof(T)) Position {
  T x;
  T _pad1 = 0;
  T y;
  T _pad2 = 0;
  T z;
  T _pad3 = 0;
  T _pad4 = 0;
  T _pad5;

  Position(T x, T y, T z) : x(x), y(y), z(z) {}

  constexpr bool operator==(Position const &other) const {
    return x == other.x and y == other.y and z == other.z;
  }

  constexpr bool operator!=(Position const &other) const {
    return !(*this == other);
  }
};

template<class T>
constexpr std::ostream &operator<<(std::ostream &oss, Position<T> position) {
  return oss << '(' << position.x << ", " << position.y << ", " << position.z << ')';
}

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input);
