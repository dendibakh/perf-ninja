use criterion::{criterion_group, criterion_main, Criterion};

use lookup_tables_1::{histogram, init};

fn bench1(c: &mut Criterion) {
    let values = init();

    // Run the benchmark
    c.bench_function("lab", |b| {
        b.iter(|| {
            let output = histogram(&values);
            std::hint::black_box(output);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
