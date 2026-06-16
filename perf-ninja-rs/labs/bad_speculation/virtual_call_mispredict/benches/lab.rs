use criterion::{criterion_group, criterion_main, Criterion};

use virtual_call_mispredict::{generate_objects, invoke};

fn bench1(c: &mut Criterion) {
    let arr = generate_objects();

    c.bench_function("lab", |b| {
        b.iter(|| {
            let mut data = 0;
            invoke(&arr, &mut data);
            std::hint::black_box(data);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
