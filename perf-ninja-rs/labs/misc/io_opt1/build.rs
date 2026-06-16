// Build script to generate the data files

use rand::prelude::*;
use std::env::var;
use std::fs::{create_dir, File};
use std::io::Write;
use std::path::{Path, PathBuf};
use std::process::ExitCode;

fn main() -> ExitCode {
    // don't re-run unless the build script changes (default is re-run on any file changes)
    println!("cargo:rerun-if-changed=build.rs");

    let root_dir = PathBuf::from(var("CARGO_MANIFEST_DIR").unwrap());
    let data_dir = root_dir.join("data");
    if !data_dir.exists() {
        create_dir(&data_dir).unwrap();
    }

    let small_data_path = data_dir.join("small.data");
    let medium_data_path = data_dir.join("medium.data");
    let large_data_path = data_dir.join("large.data");
    write_random_string(&small_data_path, 4095);
    write_random_string(&medium_data_path, 2097151);
    write_random_string(&large_data_path, 268435455);

    let path_mod = root_dir.join("src").join("data_paths.rs");
    let mut data_paths = File::create(path_mod).unwrap();
    let contents = format!(
        r#"pub const SMALL_DATA_PATH: &str = "{}";
pub const MEDIUM_DATA_PATH: &str = "{}";
pub const LARGE_DATA_PATH: &str = "{}";
"#,
        small_data_path.display(),
        medium_data_path.display(),
        large_data_path.display()
    );
    data_paths.write_all(contents.as_bytes()).unwrap();

    ExitCode::from(0)
}

fn write_random_string(path: &Path, len: usize) {
    if path.exists() {
        return;
    }
    let mut rng = thread_rng();
    /* build scripts aren't allowed output
    println!(
        "Generating file with random contents of length {len}, this may take a few seconds..."
    );
    */
    let mut f = File::create(path).unwrap();
    const CHUNK_SIZE: usize = 4096;
    let mut remain = len;
    let mut buf = [0; CHUNK_SIZE];
    while remain > CHUNK_SIZE {
        rng.fill(&mut buf[..]);
        f.write_all(&buf).unwrap();
        remain -= CHUNK_SIZE;
    }
    let mut buf = vec![];
    buf.resize(remain, 0);
    rng.fill(&mut buf[..]);
    f.write_all(&buf).unwrap();
}
