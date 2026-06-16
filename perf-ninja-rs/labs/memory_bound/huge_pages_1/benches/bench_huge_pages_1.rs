#![feature(allocator_api)]

use criterion::{criterion_group, criterion_main, Criterion, Throughput};

use huge_pages_1::{allocator, generate_mesh, solution};
use rand::distributions::{Distribution, Uniform};
use rand::rngs::StdRng;
use rand::SeedableRng;

fn bench1(c: &mut Criterion) {
    // Mesh
    const N_NODES_X: u32 = 800;
    const N_NODES_Y: u32 = 20000;
    const N_NODES: u32 = N_NODES_X * N_NODES_Y;

    const SEED: u64 = 0xaf173e8a;
    let alloc = allocator();
    let mut x = Vec::with_capacity_in(N_NODES as usize, alloc);
    let mut y = Vec::with_capacity_in(N_NODES as usize, alloc);
    x.resize(N_NODES as usize, 0.0);
    y.resize(N_NODES as usize, 0.0);
    let topology = generate_mesh(N_NODES_X, N_NODES_Y, &mut x, &mut y, SEED);

    // Generate random left-hand side
    let mut lhs = Vec::with_capacity_in(2 * N_NODES as usize, alloc);
    lhs.resize(lhs.capacity(), 0.0);
    let mut prng = StdRng::seed_from_u64(SEED);
    let dist = Uniform::from(0.0..42.0);
    for _ in 0..lhs.len() {
        lhs.push(dist.sample(&mut prng));
    }

    // Right-hand side
    let mut rhs = Vec::with_capacity_in(2 * N_NODES as usize, alloc);

    // Run the benchmark

    let mut group = c.benchmark_group("huge_pages_1");
    group.sample_size(10);
    //state.SetBytesProcessed(state.iterations() * topology.size() * 4 * sizeof(double));
    group.throughput(Throughput::Bytes(
        (topology.len() * 4 * std::mem::size_of::<f64>()) as u64,
    ));

    group.bench_function("Apply matrix-free operator", |b| {
        b.iter(|| {
            solution(&topology, N_NODES, &x, &y, &lhs, &mut rhs);
            std::hint::black_box(&rhs);
            //benchmark::ClobberMemory();
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
