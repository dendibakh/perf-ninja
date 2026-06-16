use std::fs::File;
use std::io::Read;

#[cfg(test)]
mod tests;

mod data_paths;
pub use data_paths::{LARGE_DATA_PATH, MEDIUM_DATA_PATH, SMALL_DATA_PATH};

// The following implementation is consistent with Boost's CRC32, see
// https://godbolt.org/z/shEaaE6MW
const fn generate_crc32_table(polynomial: u32) -> [u32; 256] {
    let mut a = [0u32; 256];
    let mut i = 0;
    while i < 256 {
        // for loops not allowed in const fn
        let mut v: u32 = (i as u32) << 24;
        let mut j = 0;
        while j < 8 {
            let carry = v & 0x80000000;
            v <<= 1;
            if carry != 0 {
                v ^= polynomial;
            }
            j += 1;
        }
        a[i] = v;
        i += 1;
    }
    a
}

pub fn update_crc32(crc: &mut u32, v: u8) {
    static CRC32_TABLE: [u32; 256] = generate_crc32_table(0x1EDC6F41);
    let idx = ((*crc >> 24) ^ v as u32) & 0xFF;
    *crc = (*crc << 8) ^ CRC32_TABLE[idx as usize];
}

pub fn solution(file_name: &str) -> u32 {
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
