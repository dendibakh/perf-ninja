use criterion::{criterion_group, criterion_main, Criterion};

use false_sharing_1::solution;

fn bench1(c: &mut Criterion) {
    const SIZE: usize = 1024 * 1024;

    let mut data = Vec::with_capacity(SIZE);
    let mut i: i32 = -1;
    data.resize_with(SIZE, || {
        i += 1;
        i as u32
    });

    // Use thread count from 1 to <number of HW threads>
    let max_threads = std::thread::available_parallelism().unwrap().get();
    c.bench_function("lab", |b| {
        b.iter(|| {
            for thread_count in 1..=max_threads {
                let result = solution(&data, thread_count);
                std::hint::black_box(result);
            }
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
