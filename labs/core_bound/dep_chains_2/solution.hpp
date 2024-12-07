#include <vector>
#include <iostream>
#include <cstdint>
#include <array>

#define SOLUTION

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
#ifdef SOLUTION

template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed1, uint32_t seed2) {
  RNG rng1(seed1);
  RNG rng2(seed2);
  for (int i = 0; i < STEPS; i++)
    for (int j = 0; j < particles.size() - 1; j += 2) {
      uint32_t angle1 = rng1.gen();
      float angle_rad1 = angle1 * DEGREE_TO_RADIAN;
      particles[j].x += cosine(angle_rad1) * particles[j].velocity;
      particles[j].y += sine(angle_rad1) * particles[j].velocity;
      uint32_t angle2 = rng2.gen();
      float angle_rad2 = angle2 * DEGREE_TO_RADIAN;
      particles[j+1].x += cosine(angle_rad2) * particles[j+1].velocity;
      particles[j+1].y += sine(angle_rad2) * particles[j+1].velocity;
    }
}

template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  randomParticleMotion<RNG>(particles, seed, seed+1);
}


// template <class RNG>
// void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed1, uint32_t seed2) {
//   RNG rng1(seed1);
//   RNG rng2(seed2);
//   for (int i = 0; i < STEPS; i++)
//     for (int j = 0; j < particles.size() - 1; j += 2) {
//       moveParticle(particles[j], rng1.gen());
//       moveParticle(particles[j+1], rng2.gen());
//     }
// }

// inline void moveParticle(Particle &particle, uint32_t angle) {
//   float angle_rad = angle * DEGREE_TO_RADIAN;
//   particle.x += cosine(angle_rad) * particle.velocity;
//   particle.y += sine(angle_rad) * particle.velocity;
// }

// template <class RNG>
// void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
//   randomParticleMotion<RNG>(particles, seed, seed+1);
// }

#else

template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  RNG rng(seed);  
  for (int i = 0; i < STEPS; i++)
    for (auto &p : particles) {
      uint32_t angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      p.x += cosine(angle_rad) * p.velocity;
      p.y += sine(angle_rad) * p.velocity;
    }
}
#endif