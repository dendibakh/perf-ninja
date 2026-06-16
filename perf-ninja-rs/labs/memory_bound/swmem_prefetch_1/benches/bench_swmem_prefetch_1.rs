use criterion::{criterion_group, criterion_main, Criterion};

use swmem_prefetch_1::{init, solution, HashMapT, HASH_MAP_SIZE};

fn bench1(c: &mut Criterion) {
    // Init benchmark data
    let mut hash_map = HashMapT::new(HASH_MAP_SIZE);
    let lookups = init(&mut hash_map);

    // Run the benchmark
    c.bench_function("lab", |b| {
        b.iter(|| {
            let output = solution(&hash_map, &lookups);
            std::hint::black_box(output);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
