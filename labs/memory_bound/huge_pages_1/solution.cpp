#include "solution.hpp"

#include <algorithm>
#include <cmath>

//////////////////////////////////////////////////////////////
//                       ATTENTION                          //
// You are not meant to modify this file. Please focus on   //
// AllocateDoublesArray.hpp                                 //
//////////////////////////////////////////////////////////////

auto computeLocalProduct(const std::array<double, 4> &coords,
                         const std::array<double, 4> &lhs_local)
    -> std::array<double, 4> {
  const auto [x1, y1, x2, y2] = coords;
  const auto dx = x2 - x1;
  const auto dy = y2 - y1;
  const auto dx2 = dx * dx;
  const auto dy2 = dy * dy;
  const auto dxdy = dx * dy;

  std::array<std::array<double, 4>, 4> K;
  K[0][0] = dx2;
  K[0][1] = dxdy;
  K[0][2] = -dx2;
  K[0][3] = -dxdy;
  K[1][0] = dxdy;
  K[1][1] = dy2;
  K[1][2] = -dxdy;
  K[1][3] = -dy2;
  for (unsigned c = 2; c < 4; ++c)
    for (unsigned r = 0; r < 4; ++r)
      K[c][r] = -K[c - 2][r];

  std::array<double, 4> mult_result{};
  for (unsigned c = 0; c < 4; ++c)
    for (unsigned r = 0; r < 4; ++r)
      mult_result[r] += K[c][r] * lhs_local[c];

  const auto L = std::sqrt(dx2 + dy2);
  constexpr auto E = 210e9;
  constexpr auto A = 3.14 * 1e-2 * 1e-2;
  const auto C = E * A / (L * L * L);
  for (auto &a : mult_result)
    a *= C;

  return mult_result;
}

auto computeDofs(unsigned n1, unsigned n2) -> std::array<unsigned, 4> {
  std::array<unsigned, 4> dofs;
  dofs[0] = n1 * 2;
  dofs[1] = n1 * 2 + 1;
  dofs[2] = n2 * 2;
  dofs[3] = n2 * 2 + 1;
  return dofs;
}

auto gatherGlobal(unsigned n1, unsigned n2, const double *rhs_global)
    -> std::array<double, 4> {
  const auto dofs = computeDofs(n1, n2);
  std::array<double, 4> vals;
  for (unsigned i = 0; i < dofs.size(); ++i)
    vals[i] = rhs_global[dofs[i]];
  return vals;
}

void scatterLocal(unsigned n1, unsigned n2, const std::array<double, 4> &vals,
                  double *rhs_global) {
  const auto dofs = computeDofs(n1, n2);
  for (unsigned i = 0; i < dofs.size(); ++i)
    rhs_global[dofs[i]] += vals[i];
}

// Local contrbution of the element described by the nodes (n1, n2). Remaining
// arguments are the same as the arguments of solution(...)
void processsElement(unsigned n1, unsigned n2, const double *x, const double *y,
                     const double *lhs, double *rhs) {
  const auto lhs_vals = gatherGlobal(n1, n2, lhs);
  std::array<double, 4> coords;
  coords[0] = x[n1];
  coords[1] = y[n1];
  coords[2] = x[n2];
  coords[3] = y[n2];
  const auto local_prod = computeLocalProduct(coords, lhs_vals);
  scatterLocal(n1, n2, local_prod, rhs);
}

void solution(const std::vector<std::array<unsigned, 2>> &topo,
              unsigned n_nodes, const double *x, const double *y,
              const double *lhs, double *rhs) {
  std::fill(rhs, rhs + n_nodes * 2, 0.);
  for (const auto [n1, n2] : topo)
    processsElement(n1, n2, x, y, lhs, rhs);
}
