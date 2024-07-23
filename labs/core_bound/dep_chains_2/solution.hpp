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
    XorShift32() {}
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
template<class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
    const int unroll = 4;
    alignas(RNG) char backing[sizeof(RNG) * unroll];

    RNG *rngs = (RNG *) &backing;
    for (int k = 0; k < unroll; ++k) rngs[k] = RNG(seed *= 7640183); // initialize with different seeds, just multiplying by some big prime seems to do the job

    for (int i = 0; i < STEPS; i++) {
        int j = 0;
        for (; j + unroll <= particles.size(); j += unroll) {
            for (int k = 0; k < unroll; ++k) {
                Particle &p = particles[j + k];
                uint32_t angle = rngs[k].gen();
                float angle_rad = angle * DEGREE_TO_RADIAN;
                p.x += cosine(angle_rad) * p.velocity;
                p.y += sine(angle_rad) * p.velocity;
            }
        }
        for (; j < particles.size(); ++j) {
            Particle &p = particles[j];
            uint32_t angle = rngs[0].gen();
            float angle_rad = angle * DEGREE_TO_RADIAN;
            p.x += cosine(angle_rad) * p.velocity;
            p.y += sine(angle_rad) * p.velocity;
        }
    }
}