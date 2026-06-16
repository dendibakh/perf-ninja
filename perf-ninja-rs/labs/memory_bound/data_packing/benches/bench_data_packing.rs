use criterion::{criterion_group, criterion_main, Criterion};

use data_packing::{init, solution};

fn bench1(c: &mut Criterion) {
    let mut arr = init();
    c.bench_function("lab", |b| {
        b.iter(|| {
            solution(&mut arr);
            std::hint::black_box(&mut arr);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
