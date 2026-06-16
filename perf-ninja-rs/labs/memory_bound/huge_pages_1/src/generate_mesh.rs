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

use rand::{rngs::StdRng, seq::SliceRandom, SeedableRng};

// n_nodes_x, n_nodes_y - see sketch above
// x, y - arrays where node coordinates will be written, need to have space
//        allocated for at least (n_nodes_x*n_nodes_y) doubles
// seed - seed for RNG
//
// returns topology (see lib.rs)
pub fn generate_mesh(
    n_nodes_x: u32,
    n_nodes_y: u32,
    x: &mut [f64],
    y: &mut [f64],
    seed: u64,
) -> Vec<[u32; 2]> {
    let n_nodes = n_nodes_x * n_nodes_y;

    // Topology
    let mut topology: Vec<[u32; 2]> = Vec::with_capacity(
        ((n_nodes_x - 1) * (n_nodes_y - 1) * 3 + (n_nodes_x - 1) + (n_nodes_y - 1)) as usize,
    );
    for j in 0..(n_nodes_y - 1) {
        for i in 0..(n_nodes_x - 1) {
            let base = n_nodes_x * j + i;
            topology.push([base, base + 1]);
            topology.push([base, base + n_nodes_x]);
            topology.push([base, base + n_nodes_x + 1]);
        }
        topology.push([n_nodes_x * (j + 1) - 1, n_nodes_x * (j + 2) - 1]);
    }
    for i in 0..(n_nodes_x - 1) {
        topology.push([
            n_nodes_x * (n_nodes_y - 1) + i,
            n_nodes_x * (n_nodes_y - 1) + i + 1,
        ]);
    }

    // Node coords
    let mut x_unshuffled = vec![0.0; n_nodes as usize];
    let mut y_unshuffled = vec![0.0; n_nodes as usize];
    let mut coord_ind: usize = 0;
    for j in 0..n_nodes_y {
        for i in 0..n_nodes_x {
            x_unshuffled[coord_ind] = i as f64;
            y_unshuffled[coord_ind] = j as f64;
            coord_ind += 1;
        }
    }

    // Shuffle
    let mut prng = StdRng::seed_from_u64(seed);
    let mut permutation: Vec<f64> = (0..n_nodes).map(|x| x as f64).collect();
    permutation.shuffle(&mut prng);
    x[..n_nodes as usize].copy_from_slice(&x_unshuffled[..n_nodes as usize]);
    y[..n_nodes as usize].copy_from_slice(&y_unshuffled[..n_nodes as usize]);

    for [n1, n2] in topology.iter_mut() {
        *n1 = permutation[*n1 as usize] as u32;
        *n2 = permutation[*n2 as usize] as u32;
    }
    topology.shuffle(&mut prng);

    topology
}
