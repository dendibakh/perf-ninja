#include <vector>
#include <iostream>
#include <cstdint>
#include <array>
// #include <immintrin.h>

// The number of motion simulation steps.
constexpr uint32_t STEPS = 10000;
// The number of paticles to simulate.
constexpr uint32_t PARTICLES = 1000;

struct Particle {
  float x;
  float y;
  float velocity;
};

// Initialize the particles with random coordinates and velocities.
std::vector<Particle> initParticles();

// Medium-quality random number generator.
// https://www.javamex.com/tutorials/random_numbers/xorshift.shtml
struct XorShift32 {
  uint32_t val;
  XorShift32 (uint32_t seed) : val(seed) {}
public:
  uint32_t gen() {
    val ^= (val << 13);
    val ^= (val >> 17);
    val ^= (val << 5);
    return val;
  }
};

constexpr double PI_D = 3.141592653589793238463;
constexpr float PI_F = 3.14159265358979f;

// Approximate sine and cosine functions
// https://stackoverflow.com/questions/18662261/fastest-implementation-of-sine-cosine-and-square-root-in-c-doesnt-need-to-b
static float sine(float x) {
    const float B = 4 / PI_F;
    const float C = -4/( PI_F * PI_F);
    return B * x + C * x * std::abs(x);
}
static float cosine(float x) {
    return sine(x + (PI_F / 2));
}

// static __m256 abs(__m256 x) {
//   static const __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
//   return _mm256_and_ps(x, abs_mask);
// }

// static __m256 sine(__m256 x) {
//   static const __m256 B = _mm256_set1_ps(4.0 / PI_F);
//   static const __m256 C = _mm256_set1_ps(-4.0 / (PI_F * PI_F));
//   const __m256 abs_x = abs(x); 
//   __m256 y = _mm256_mul_ps(B, x);
//   __m256 z = _mm256_mul_ps(C, x);
//   z = _mm256_mul_ps(z, abs_x);
//   return _mm256_add_ps(y, z);
// }

// static __m256 cosine(__m256 x) {
//   static const __m256 B = _mm256_set1_ps(PI_F / 2);
//   x = _mm256_add_ps(x, B);
//   return sine(x);
// }

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;
// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  static constexpr int kBatch = 8;
  std::vector<RNG> rngs;
  for (int i = 0; i < kBatch; i++) {
    rngs.emplace_back(seed + i);
  }
  for (int i = 0; i < STEPS; i++) {
    for (int j = 0; j + kBatch - 1 < particles.size(); j += kBatch) {
      for (int z = 0; z < kBatch; z++) {
        uint32_t angle1 = rngs[z].gen(); 
        float angle_rad1 = angle1 * DEGREE_TO_RADIAN;
        particles[j + z].x += cosine(angle_rad1) * particles[j + z].velocity;
        particles[j + z].y += sine(angle_rad1)   * particles[j + z].velocity;
      }
    }
  } 
}