#include <array>
#include <cassert>
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
  XorShift32() = default;

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
inline static float sine(float x) {
  const float B = 4 / PI_F;
  const float C = -4 / (PI_F * PI_F);
  return B * x + C * x * std::abs(x);
}
inline static float cosine(float x) { return sine(x + (PI_F / 2)); }

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;

// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  constexpr int CHUNK_SIZE = 8;
  assert(particles.size() % CHUNK_SIZE == 0 &&
         "The number of particles must be a multiple of CHUNK_SIZE");
  std::array<RNG, CHUNK_SIZE> rngs;
  for (int i = 0; i < CHUNK_SIZE; ++i) {
    rngs[i] = RNG(seed + i);
  }
  std::array<uint32_t, CHUNK_SIZE> angles;
  for (int step = 0; step < STEPS; step++) {
    int i = 0;
    for (; i < particles.size(); i += CHUNK_SIZE) {
      for (int j = 0; j < CHUNK_SIZE; ++j) {
        uint32_t angle = rngs[j].gen();
        float angle_rad = angle * DEGREE_TO_RADIAN;
        particles[i + j].x += cosine(angle_rad) * particles[i + j].velocity;
        particles[i + j].y += sine(angle_rad) * particles[i + j].velocity;
      }
    }
  }
}
