use crate::{solution, update_crc32, SMALL_DATA_PATH};
use std::fs::File;
use std::io::Read;

#[test]
fn validate() {
    let original_result = original_solution(SMALL_DATA_PATH);
    let result = solution(SMALL_DATA_PATH);
    assert_eq!(original_result, result);
}

fn original_solution(file_name: &str) -> u32 {
    let mut file_stream = match File::open(file_name) {
        Ok(fs) => fs,
        Err(err) => {
            panic!("The file could not be opened: {err}");
        }
    };

    // Initial value has all bits set to 1
    let mut crc: u32 = 0xff_ff_ff_ff;

    // Update the CRC32 value character by character
    let mut buf = [0u8; 1];
    loop {
        let bytes_read = file_stream.read(&mut buf).unwrap();
        if bytes_read == 0 {
            break;
        }
        update_crc32(&mut crc, buf[0]);
    }

    // Invert the bits
    crc ^= 0xff_ff_ff_ff;

    crc
}
