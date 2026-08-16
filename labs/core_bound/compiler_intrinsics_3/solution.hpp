#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

template<class T>
struct alignas(8 * sizeof(T)) Position {
  T x;
  char _pad1[sizeof(T)] = {};
  T y;
  char _pad2[sizeof(T)] = {};
  T z;
  char _pad3[sizeof(T)] = {};

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
