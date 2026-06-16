use criterion::{criterion_group, criterion_main, Criterion};

use loop_interchange_2::{blur, Grayscale, K_MAX_IMAGE_DIMENSION};

fn bench1(c: &mut Criterion) {
    let mut image: Grayscale = Default::default();
    if !image.load("pexels-pixabay-434334.pbm", K_MAX_IMAGE_DIMENSION) {
        // Original message: An IO problem. C++ programmers lol amirite?
        panic!("Load of pexels-pixabay-434334.pbm failed");
    }

    let mut result = vec![0u8; image.size];
    let mut temp = vec![0u8; image.size];
    c.bench_function("lab", |b| {
        b.iter(|| {
            blur(
                result.as_mut_slice(),
                image.data.as_slice(),
                image.width,
                image.height,
                temp.as_mut_slice(),
            );

            std::hint::black_box(&mut image);
        });
    });
}

criterion_group!(benches, bench1);
criterion_main!(benches);
