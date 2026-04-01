#include <vector>
#include <iostream>
#include <cstdint>
#include <array>

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

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;

// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  RNG rng0(seed), rng1(seed+1), rng2(seed+2), rng3(seed+3);
  int sz = particles.size();
  int j = 0;
  for (int i = 0; i < STEPS; i++) {
    j = 0;
    for (; j + 3 < sz ; j += 4) {
      auto &p0 = particles[j], &p1 = particles[j+1], &p2 = particles[j+2], &p3 = particles[j+3];
      uint32_t angle0 = rng0.gen(), angle1 = rng1.gen(), angle2 = rng2.gen(), angle3 = rng3.gen();
      float angle_rad0 = angle0 * DEGREE_TO_RADIAN;
      float angle_rad1 = angle1 * DEGREE_TO_RADIAN;
      float angle_rad2 = angle2 * DEGREE_TO_RADIAN;
      float angle_rad3 = angle3 * DEGREE_TO_RADIAN;
      p0.x += cosine(angle_rad0) * p0.velocity;
      p0.y += sine(angle_rad0) * p0.velocity;
      p1.x += cosine(angle_rad1) * p1.velocity;
      p1.y += sine(angle_rad1) * p1.velocity;
      p2.x += cosine(angle_rad2) * p2.velocity;
      p2.y += sine(angle_rad2) * p2.velocity;
      p3.x += cosine(angle_rad3) * p3.velocity;
      p3.y += sine(angle_rad3) * p3.velocity;
    }
  }
}