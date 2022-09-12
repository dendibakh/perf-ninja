#include "GenerateMesh.hpp"

#include <algorithm>
#include <numeric>

auto generateMesh(unsigned n_nodes_x, unsigned n_nodes_y, double *x, double *y,
                  unsigned seed) -> std::vector<std::array<unsigned, 2>> {
  const unsigned n_nodes = n_nodes_x * n_nodes_y;

  // Topology
  std::vector<std::array<unsigned, 2>> topology;
  topology.reserve((n_nodes_x - 1) * (n_nodes_y - 1) * 3 + (n_nodes_x - 1) +
                   (n_nodes_y - 1));
  for (unsigned j = 0; j < n_nodes_y - 1; ++j) {
    for (unsigned i = 0; i < n_nodes_x - 1; ++i) {
      const unsigned base = n_nodes_x * j + i;
      topology.push_back(std::array<unsigned, 2>{base, base + 1});
      topology.push_back(std::array<unsigned, 2>{base, base + n_nodes_x});
      topology.push_back(std::array<unsigned, 2>{base, base + n_nodes_x + 1});
    }
    topology.push_back(std::array<unsigned, 2>{n_nodes_x * (j + 1) - 1,
                                               n_nodes_x * (j + 2) - 1});
  }
  for (unsigned i = 0; i < n_nodes_x - 1; ++i)
    topology.push_back(std::array<unsigned, 2>{
        n_nodes_x * (n_nodes_y - 1) + i, n_nodes_x * (n_nodes_y - 1) + i + 1});

  // Node coords
  std::vector<double> x_unshuffled(n_nodes);
  std::vector<double> y_unshuffled(n_nodes);
  unsigned coord_ind = 0;
  for (unsigned j = 0; j < n_nodes_y; ++j)
    for (unsigned i = 0; i < n_nodes_x; ++i) {
      x_unshuffled[coord_ind] = i;
      y_unshuffled[coord_ind] = j;
      ++coord_ind;
    }

  // Shuffle
  std::mt19937 prng{seed};
  std::vector<unsigned> permutation(n_nodes);
  std::iota(permutation.begin(), permutation.end(), 0u);
  std::shuffle(permutation.begin(), permutation.end(), prng);
  unsigned i = 0;
  for (auto p : permutation) {
    x[p] = x_unshuffled[i];
    y[p] = y_unshuffled[i];
    ++i;
  }
  for (auto &[n1, n2] : topology) {
    n1 = permutation[n1];
    n2 = permutation[n2];
  }
  std::shuffle(topology.begin(), topology.end(), prng);
  return topology;
}
