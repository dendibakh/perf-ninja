use crate::{allocator, generate_mesh, solution};
use rand::distributions::{Distribution, Uniform};
use std::alloc::{Allocator, System};

const N_NODES_X: u32 = 100;
const N_NODES_Y: u32 = 200;
const N_NODES: u32 = N_NODES_X * N_NODES_Y;

fn solve<A: Allocator>(alloc: &'static A) -> Vec<f64, &'static A> {
    let mut x = Vec::with_capacity_in(N_NODES as usize, alloc);
    let mut y = Vec::with_capacity_in(N_NODES as usize, alloc);
    x.resize(N_NODES as usize, 0.0);
    y.resize(N_NODES as usize, 0.0);
    let topology = generate_mesh(N_NODES_X, N_NODES_Y, &mut x, &mut y, 0);

    // Generate random left-hand side
    let mut lhs = Vec::with_capacity_in(2 * N_NODES as usize, alloc);
    lhs.resize(2 * N_NODES as usize, 0.0);

    let mut prng = rand::thread_rng();
    let dist = Uniform::from(0.0..42.0);
    for _ in 0..lhs.len() {
        lhs.push(dist.sample(&mut prng));
    }

    // Right-hand side
    let mut rhs = Vec::with_capacity_in(2 * N_NODES as usize, alloc);

    // Eval operator
    solution(&topology, N_NODES, &x, &y, &lhs, &mut rhs);

    rhs
}

#[test]
fn validate() {
    let sol_user = solve(allocator());
    let sol_valid = solve(&System);

    let mut acc = 0.0;
    for i in 0..sol_valid.len() {
        let a = sol_valid[i];
        let b = sol_user[i];
        acc += (a - b) * (a - b);
    }
    let l2_error = f64::sqrt(acc);

    assert!(l2_error <= 1e-9);
}
