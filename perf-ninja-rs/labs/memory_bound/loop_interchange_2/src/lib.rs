use std::cmp::{max, min};
use std::fs::File;
use std::io::{BufRead, BufReader, Read, Write};

#[cfg(test)]
mod tests;

pub const K_MAX_IMAGE_DIMENSION: usize = 32 * 1024;

// Applies Gaussian blur in independent vertical lines
// Originally `static void filterVertically(..`. What does 'static' mean here?
fn filter_vertically(
    output: &mut [u8],
    input: &[u8],
    width: usize,
    height: usize,
    kernel: &[i32],
    radius: usize,
    shift: usize,
) {
    let rounding = 1 << (shift - 1);

    for c in 0..width {
        // Top part of line, partial kernel
        for r in 0..min(radius, height) {
            // Accumulation
            let mut dot = 0;
            let mut sum = 0;
            let mut p = radius - r;
            for y in 0..=min(r + radius, height - 1) {
                let weight = kernel[p];
                p += 1;
                dot += input[y * width + c] as i32 * weight;
                sum += weight;
            }

            // Normalization
            // dot/sum fits in a u8, so f32 -> u8 is reasonable here
            let value = dot as f32 / sum as f32 + 0.5;
            output[r * width + c] = value as u8;
        }

        // Middle part of computations with full kernel
        for r in radius..(height - radius) {
            // Accumulation
            let mut dot = 0;
            for i in 0..(radius + 1 + radius) {
                dot += input[(r - radius + i) * width + c] as i32 * kernel[i];
            }

            // Fast shift instead of division
            let value: i32 = (dot + rounding) >> shift;
            output[r * width + c] = value as u8;
        }

        // Bottom part of line, partial kernel
        for r in std::cmp::max(radius, height - radius)..height {
            // Accumulation
            let mut dot = 0;
            let mut sum = 0;
            let mut p = 0;
            for y in (r - radius)..height {
                let weight = kernel[p];
                p += 1;
                dot += input[y * width + c] as i32 * weight;
                sum += weight;
            }

            // Normalization
            let value = dot as f32 / sum as f32 + 0.5;
            output[r * width + c] = value as u8;
        }
    }
}

// Applies Gaussian blur in independent horizontal lines
fn filter_horizontally(
    output: &mut [u8],
    input: &[u8],
    width: usize,
    height: usize,
    kernel: &[i32],
    radius: usize,
    shift: usize,
) {
    let rounding = 1 << (shift - 1);

    for r in 0..height {
        // Left part of line, partial kernel
        for c in 0..min(radius, width) {
            // Accumulation
            let mut dot = 0;
            let mut sum = 0;
            let mut p = radius - c;
            for x in 0..=min(c + radius, width - 1) {
                let weight = kernel[p];
                p += 1;
                dot += input[r * width + x] as i32 * weight;
                sum += weight;
            }

            // Normalization
            let value = dot as f32 / sum as f32 + 0.5;
            output[r * width + c] = value as u8;
        }

        // Middle part of computations with full kernel
        for c in radius..(width - radius) {
            // Accumulation
            let mut dot = 0;
            for i in 0..(radius + 1 + radius) {
                dot += input[r * width + c - radius + i] as i32 * kernel[i];
            }

            // Fast shift instead of division
            let value = (dot + rounding) >> shift;
            output[r * width + c] = value as u8;
        }

        // Right part of line, partial kernel
        for c in max(radius, width - radius)..width {
            // Accumulation
            let mut dot = 0;
            let mut sum = 0;
            let mut p = 0;
            for x in (c - radius)..width {
                let weight = kernel[p];
                p += 1;
                dot += input[r * width + x] as i32 * weight;
                sum += weight;
            }

            // Normalization
            let value = dot as f32 / sum as f32 + 0.5;
            output[r * width + c] = value as u8;
        }
    }
}

// Applies Gaussian blur to a grayscale image
pub fn blur(output: &mut [u8], input: &[u8], width: usize, height: usize, temp: &mut [u8]) {
    // Integer Gaussian blur with kernel size 5
    // https://en.wikipedia.org/wiki/Kernel_(image_processing)
    const RADIUS: usize = 2;
    const KERNEL: [i32; 5] = [1, 4, 6, 4, 1];
    // An alternative to division by power of two = sum(kernel)
    const SHIFT: usize = 4;

    // A pair of 1-dimensional passes to achieve 2-dimensional transform
    filter_vertically(temp, input, width, height, &KERNEL, RADIUS, SHIFT);
    filter_horizontally(output, temp, width, height, &KERNEL, RADIUS, SHIFT);
}

#[derive(Default)]
pub struct Grayscale {
    pub data: Vec<u8>,
    pub size: usize,
    pub width: usize,
    pub height: usize,
}

// Loads grayscale image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html Function doesn't
// support comments.
impl Grayscale {
    pub fn load(&mut self, filename: &str, max_size: usize) -> bool {
        self.data.clear();

        let f = File::open(filename).unwrap();
        let mut input = BufReader::new(f);

        let mut line = String::new();
        input.read_line(&mut line).unwrap();

        if line.trim_end() == "P5" {
            // C++ original: input >> width >> height >> amplitude;
            line.clear();
            input.read_line(&mut line).unwrap();

            let mut dim = line.split_whitespace();
            self.width = dim.next().unwrap().parse().unwrap();
            // the format is either "width\nheight" or "width height"
            self.height = match dim.next() {
                Some(h) => h.parse().unwrap(),
                None => {
                    line.clear();
                    input.read_line(&mut line).unwrap();
                    line.trim_end().parse().unwrap()
                }
            };

            line.clear();
            input.read_line(&mut line).unwrap();

            let amplitude: i32 = line.trim_end().parse().unwrap();

            // the C++ version now reads a byte expecting a '\n'
            // we already consume that in `read_line` above

            if (self.width > 0)
                && (self.width <= max_size)
                && (self.height > 0)
                && (self.height <= max_size)
                && (amplitude >= 0)
                && (amplitude <= 255)
            {
                self.size = self.width * self.height;
                self.data.resize(self.size, 0);
                if input.read_exact(&mut self.data[0..self.size]).is_err() {
                    self.data.clear();
                }
            }
        }

        !self.data.is_empty()
    }

    // Saves grayscale image. Format is
    // https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html
    pub fn save(&self, filename: &str) -> std::io::Result<()> {
        let mut output = File::create(filename).unwrap();
        write!(output, "P5\n{} {}\n255\n", self.width, self.height)?;

        if !self.data.is_empty() {
            output.write_all(self.data.as_slice())?
        }
        Ok(())
    }
}
