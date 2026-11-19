//! Rust half of LibMatti: the UI.
//!
//! C owns the API - the bootstrap launcher, the mod loader, the early window,
//! the render backends, the block/item registries and the texture/font
//! resources. This crate owns the game window after the early window and every
//! screen, and draws through the C bridge in `libmatti/matti/ui/bridge.h`.
//!
//! The exported surface is declared in `libmatti/rs/matti_rs.h` - keep both
//! sides in sync.

mod ui;

use std::ffi::CStr;
use std::os::raw::{c_char, c_int};
use std::panic::{catch_unwind, AssertUnwindSafe};

/// Entry point called from C (`LIBMATTI_MC_Client_Main_Main`, after the
/// launcher chain and the early window).
///
/// Returns the exit status the C main propagates: 0 on success, non-zero if the
/// Rust side failed. A panic is caught here so it never unwinds into C.
#[no_mangle]
pub extern "C" fn matti_rs_init(argc: c_int, argv: *const *const c_char) -> c_int {
    catch_unwind(AssertUnwindSafe(|| run(argc, argv))).unwrap_or(1)
}

fn run(argc: c_int, argv: *const *const c_char) -> c_int {
    if argc < 0 || (argc > 0 && argv.is_null()) {
        eprintln!("matti-rs: invalid argv passed from C");
        return 2;
    }

    let args = unsafe { collect_args(argc, argv) };
    ui::run(&args)
}

/// Copies the C `argv` into owned strings. The pointers come from C, so each
/// one is checked before it is dereferenced.
unsafe fn collect_args(argc: c_int, argv: *const *const c_char) -> Vec<String> {
    let mut args = Vec::with_capacity(argc as usize);
    for index in 0..argc as isize {
        let arg = *argv.offset(index);
        if arg.is_null() {
            continue;
        }
        args.push(CStr::from_ptr(arg).to_string_lossy().into_owned());
    }
    args
}
