use std::alloc::{Allocator, System};

// Replace the body of this function with a huge pages allocator
pub fn allocator() -> &'static impl Allocator {
    &System
}
