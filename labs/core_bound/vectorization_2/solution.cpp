#include "solution.hpp"

uint16_t checksum(const blob &blob) {
  uint32_t acc = 0;
  for (const auto value : blob) {
    acc += value;
  }

  auto high = acc >> 16;
  auto low = acc & 0xFFFFu;
  acc = low + high;

  return static_cast<uint16_t>(acc);
}

// uint16_t checksum_chunked(const blob &blob) {
//   constexpr size_t step = 4;
//   std::array<uint16_t, step> acc{};
//
//   size_t n = blob.size();
//   size_t aligned = n - (n % step);
//
//   for (size_t i = 0; i < aligned; i += step) {
//     for (size_t j = 0; j < step; ++j) {
//       uint16_t value = blob[i + j];
//       acc[j] += value;
//       acc[j] += acc[j] < value;
//     }
//   }
//
//   // remainder
//   for (size_t i = aligned; i < n; ++i) {
//     uint16_t value = blob[i];
//     acc[0] += value;
//     acc[0] += acc[0] < value;
//   }
//
//   uint16_t final_acc = 0;
//   for (auto value : acc) {
//     final_acc += value;
//     final_acc += final_acc < value;
//   }
//   return final_acc;
// }
// 
// this didn't end up getting vectorized, the modulus was the killer
// the compiler can't figure out the access pattern statically
// uint16_t checksum_interleaved(const blob &blob) {
//   constexpr size_t step = 4;
//   std::array<uint16_t, step> acc{};
//
//   for (size_t i = 0; i < blob.size(); ++i) {
//     uint16_t value = blob[i];
//     acc[i % step] += value;
//     acc[i % step] += acc[i % step] < value;
//   }
//
//   uint16_t final_acc = 0;
//   for (auto value : acc) {
//     final_acc += value;
//     final_acc += final_acc < value;
//   }
//   return final_acc;
// }
//
// uint16_t checksum_unoptimized_original(const blob &blob) {
//   uint32_t acc = 0;
//   for (const auto value : blob) {
//     acc += value;
//   }
//   auto high = acc >> 16;
//   auto low = acc & 0xFFFFu;
//   acc = low + high;
//   return static_cast<uint16_t>(acc);
// }
