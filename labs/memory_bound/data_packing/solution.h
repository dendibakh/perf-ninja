#pragma once

#include <array>
#include <cstdint>

// Assume those constants never change
constexpr int N = 10000;
constexpr int minRandom = 0;
constexpr int maxRandom = 100;

struct S {
  float d;         // {0, 0.1, 1}
  uint16_t l : 14; // [0, 10000]
  uint16_t b : 2;  // [0..1]
  uint8_t i;       // [0..100]
  uint8_t s;       // [0..100]

  bool operator<(const S &s) const { return this->i < s.i; }
};

void init(std::array<S, N> &arr);
S create_entry(int first_value, int second_value);
void solution(std::array<S, N> &arr);
