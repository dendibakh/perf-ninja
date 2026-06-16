//////////////////////////////////////////////////////////////
//                       ATTENTION                          //
// You are not meant to modify this file. Please focus on   //
// allocator.rs                                             //
//////////////////////////////////////////////////////////////

#![feature(maybe_uninit_array_assume_init)]
#![feature(allocator_api)]

use std::alloc::Allocator;
use std::mem::MaybeUninit;

mod allocator;
pub use allocator::allocator;
mod generate_mesh;
pub use generate_mesh::generate_mesh;

#[cfg(test)]
mod tests;

fn compute_local_product(coords: &[f64; 4], &lhs_local: &[f64; 4]) -> [f64; 4] {
    let dx: f64 = coords[2] - coords[0];
    let dy: f64 = coords[3] - coords[1];
    let dx2: f64 = dx * dx;
    let dy2: f64 = dy * dy;
    let dxdy: f64 = dx * dy;

    let mut k = [
        [dx2, dxdy, -dx2, -dxdy],
        [dxdy, dy2, -dxdy, -dy2],
        [0f64; 4],
        [0f64; 4],
    ];
    let mut c = 2;
    while c < 4 {
        let mut r = 0;
        while r < 4 {
            k[c][r] = -k[c - 2][r];
            r += 1;
        }
        c += 1;
    }
    let mut mult_result = [0f64; 4];
    let mut c = 0;
    while c < 4 {
        let mut r = 0;
        while r < 4 {
            mult_result[r] += k[c][r] * lhs_local[c];
            r += 1;
        }
        c += 1;
    }
    let l = f64::sqrt(dx2 + dy2);
    const E: f64 = 210e9;
    #[allow(clippy::approx_constant)]
    const A: f64 = 3.14 * 1e-2 * 1e-2;
    let c = E * A / (l * l * l);
    let mut ai = 0;
    while ai < mult_result.len() {
        mult_result[ai] *= c;
        ai += 1;
    }
    mult_result
}

const fn compute_dofs(n1: u32, n2: u32) -> [u32; 4] {
    let mut dofs = [0u32; 4];
    dofs[0] = n1 * 2;
    dofs[1] = n1 * 2 + 1;
    dofs[2] = n2 * 2;
    dofs[3] = n2 * 2 + 1;
    dofs
}

fn gather_global(n1: u32, n2: u32, rhs_global: &[f64]) -> [f64; 4] {
    let dofs = compute_dofs(n1, n2);
    let mut vals: [MaybeUninit<f64>; 4] = std::array::from_fn(|_| MaybeUninit::uninit());
    let mut i = 0;
    while i < dofs.len() {
        vals[i].write(rhs_global[dofs[i] as usize]);
        i += 1;
    }
    unsafe { MaybeUninit::array_assume_init(vals) }
}

fn scatter_local(n1: u32, n2: u32, vals: &[f64; 4], rhs_global: &mut [f64]) {
    let dofs = compute_dofs(n1, n2);
    let mut i = 0;
    while i < dofs.len() {
        rhs_global[dofs[i] as usize] += vals[i];
        i += 1;
    }
}

// Local contrbution of the element described by the nodes (n1, n2). Remaining
// arguments are the same as the arguments of solution(...)
fn processs_element(n1: u32, n2: u32, x: &[f64], y: &[f64], lhs: &[f64], rhs: &mut [f64]) {
    let lhs_vals: [f64; 4] = gather_global(n1, n2, lhs);
    let mut coords = [0f64; 4];
    coords[0] = x[n1 as usize];
    coords[1] = y[n1 as usize];
    coords[2] = x[n2 as usize];
    coords[3] = y[n2 as usize];
    let local_prod = compute_local_product(&coords, &lhs_vals);
    scatter_local(n1, n2, &local_prod, rhs);
}

// Evaluate matrix-free operator for a 2D truss
//
// topo - topology of the mesh. Each entry in the vector represents a single
//        element, described by the 2 IDs of the nodes of the element.
// n_nodes - total number of nodes in the mesh
// x, y - arrays containing the coordinates of the nodes - i-th entry contains
//        the coordinates of the i-th node
// lhs - left-hand side vector - this is the vector which is to be multiplied by
//       the stiffness matrix. It has a length of 2 * n_nodes (2 DOFs per node)
// rhs - right-hand side - this is the vector where we want to write the result
//       of the multiplication (same length as lhs)
pub fn solution<A: Allocator>(
    topo: &[[u32; 2]],
    n_nodes: u32,
    x: &[f64],
    y: &[f64],
    lhs: &[f64],
    rhs: &mut Vec<f64, A>,
) {
    rhs.resize(n_nodes as usize * 2, 0.0);
    for [n1, n2] in topo {
        processs_element(*n1, *n2, x, y, lhs, rhs);
    }
}
