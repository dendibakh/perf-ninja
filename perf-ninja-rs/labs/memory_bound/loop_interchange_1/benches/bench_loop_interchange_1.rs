use criterion::{criterion_group, criterion_main, Criterion};

use loop_interchange_1::{create_matrix, init, power, zero};

fn bench1(c: &mut Criterion) {
    let mut matrix_a = create_matrix();
    init(&mut matrix_a);
    let mut matrix_b = create_matrix();
    zero(&mut matrix_b);

    c.bench_function("lab", |b| {
        b.iter(|| {
            matrix_b = power(&matrix_a, 2021);
            std::hint::black_box(&mut matrix_b);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
