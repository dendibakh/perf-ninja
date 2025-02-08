#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

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
  XorShift32(uint32_t seed) : val(seed) {}

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
  const float C = -4 / (PI_F * PI_F);
  return B * x + C * x * std::abs(x);
}
static float cosine(float x) { return sine(x + (PI_F / 2)); }

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;

// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
#define NRNG 8
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  std::array<RNG, NRNG> rng{{RNG(seed), RNG(seed + 1), RNG(seed + 2),
                             RNG(seed + 3), RNG(seed + 4), RNG(seed + 5),
                             RNG(seed + 6), RNG(seed + 7)}};
  for (int i = 0; i < STEPS; i++) {
    unsigned int j = 0;
    for (; j + NRNG < particles.size(); j += NRNG) {
      for (int k = 0; k < NRNG; ++k) {
        uint32_t angle = rng[k].gen();
        float angle_rad = angle * DEGREE_TO_RADIAN;
        particles[j + k].x += cosine(angle_rad) * particles[j + k].velocity;
        particles[j + k].y += sine(angle_rad) * particles[j + k].velocity;
      }
    }
    for (int k = 0; j + k < particles.size(); ++k) {
      uint32_t angle = rng[k].gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      particles[j + k].x += cosine(angle_rad) * particles[j + k].velocity;
      particles[j + k].y += sine(angle_rad) * particles[j + k].velocity;
    }
  }
}
