#include <array>
#include <random>
#include <vector>

// Generate an example mesh describing a 2D truss.
// In this example, the nodes are distributed on a cartesian grid, the topology
// of the mesh is as follows:
/*
 * o-o-o-o-o-o  ^
 * |\|\|\|\|\|  |
 * o-o-o-o-o-o  |  n_nodes_y layers
 * |\|\|\|\|\|  |
 * o-o-o-o-o-o  |
 *
 * ----------->
 * n_nodes_x layers
 *
 */
// n_nodes_x, n_nodes_y - see sketch above
// x, y - arrays where node coordinates will be written, need to have space
//        allocated for at least (n_nodes_x*n_nodes_y) doubles
// seed - seed for RNG
//
// returns topology (see solution.hpp)
auto generateMesh(unsigned n_nodes_x, unsigned n_nodes_y, double *x, double *y,
                  unsigned seed = std::random_device{}())
    -> std::vector<std::array<unsigned, 2>>;
