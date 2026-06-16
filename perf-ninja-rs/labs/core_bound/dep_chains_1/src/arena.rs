use crate::{List, N};
use core::alloc::{AllocError, Allocator, Layout};
use std::cell::RefCell;
use std::ptr::{null, slice_from_raw_parts_mut, NonNull};

const LIST_SIZE: usize = std::mem::size_of::<List>();

// Simplest one-time arena allocator
// Allocate List objects consecutively.
#[repr(align(128))]
pub struct Arena {
    arena: [u8; N * LIST_SIZE],

    // RefCell because Allocator::allocator() takes &self, not &mut self
    cur_ptr: RefCell<*const u8>,
    end_ptr: RefCell<*const u8>,
}

impl Arena {
    pub fn new() -> Arena {
        let a = Arena {
            arena: [0; N * LIST_SIZE],
            cur_ptr: RefCell::new(null()),
            end_ptr: RefCell::new(null()),
        };
        a.cur_ptr.replace(a.arena.as_ptr());
        a.end_ptr
            .replace(unsafe { a.cur_ptr.borrow().add(a.arena.len()) });
        a
    }
}

unsafe impl Allocator for Arena {
    fn allocate(&self, layout: Layout) -> Result<NonNull<[u8]>, AllocError> {
        let cur_ptr = *self.cur_ptr.borrow_mut();
        let end_ptr = *self.end_ptr.borrow();
        let space = end_ptr as usize - cur_ptr as usize;
        if space < layout.size() {
            return Err(AllocError {});
        }
        let mem_ptr = slice_from_raw_parts_mut(cur_ptr as *mut u8, layout.size());
        unsafe {
            self.cur_ptr.replace(cur_ptr.add(layout.size()));
            Ok(NonNull::new_unchecked(mem_ptr))
        }
    }

    unsafe fn deallocate(&self, _ptr: NonNull<u8>, _layout: Layout) {
        // no dealloc method - we hold Arena for duration of benchmark
    }
}
