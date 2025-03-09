#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

template<class T>
struct Position {
  T x;
  T y;
  T z;

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
