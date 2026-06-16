use criterion::{criterion_group, criterion_main, Criterion};

use compiler_intrinsics_1::{image_smoothing, init, OutputVector, RADIUS};

fn bench1(c: &mut Criterion) {
    let in_a = init();
    let mut out_b: OutputVector = vec![0; in_a.len()];

    c.bench_function("lab", |b| {
        b.iter(|| {
            image_smoothing(&in_a, RADIUS, &mut out_b);
            std::hint::black_box(&mut out_b);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
