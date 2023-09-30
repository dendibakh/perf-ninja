#include "solution.hpp"
#include <algorithm>
#include <random>

std::vector<Particle> initParticles() {
  std::random_device r;
  std::default_random_engine gen(r());
  // particles are moving with a constant speed in the range [0;1]
  std::uniform_real_distribution<float> distrib_velocity(0.0f, 1.0f);
  // particles have initial x and y coordinates in the range [-1000,1000]
  std::uniform_real_distribution<float> distrib_coord(-1000.0f, 1000.0f);
  std::vector<Particle> particles;
  particles.reserve(PARTICLES);
  for (int i = 0; i < PARTICLES; i++)
    particles.push_back({distrib_coord(gen), distrib_coord(gen), distrib_velocity(gen)});
  return particles;
}