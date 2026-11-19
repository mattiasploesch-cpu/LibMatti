//! The C bridge (`libmatti/matti/ui/bridge.h`, `menu.h`, `texture.h`).
//!
//! Rust only drives the window: open/close, event pump, present and the C menu
//! state. Drawing, texturing and input handling live behind these functions.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_uint};

#[repr(C)]
pub struct WindowConfig {
    pub width: c_int,
    pub height: c_int,
    pub fullscreen: c_int,
    pub vsync: c_int,
    pub selection: *const c_char,
    pub title: *const c_char,
}

extern "C" {
    fn LIBMATTI_UI_WindowOpen(config: *const WindowConfig) -> c_int;
    fn LIBMATTI_UI_WindowClose();
    fn LIBMATTI_UI_LastError() -> *const c_char;

    fn LIBMATTI_UI_PollEvents();
    fn LIBMATTI_UI_ShouldClose() -> c_int;
    fn LIBMATTI_UI_Time() -> f64;

    fn LIBMATTI_UI_Present();
    fn LIBMATTI_UI_SaveScreenshot(path: *const c_char) -> c_int;

    fn LIBMATTI_UI_MainWindow_Reset();
    fn LIBMATTI_UI_MainWindow_SetScreenByName(name: *const c_char);
    fn LIBMATTI_UI_MainWindow_Update();
    fn LIBMATTI_UI_MainWindow_Draw(time: f64);
    fn LIBMATTI_UI_MainWindow_ShouldClose() -> c_int;
    fn LIBMATTI_UI_MainWindow_TakeRestart(
        selection: *mut c_char,
        selection_size: c_int,
        fullscreen: *mut c_int,
    ) -> c_int;



    fn LIBMATTI_UI_WindowWidth() -> c_int;
    fn LIBMATTI_UI_WindowHeight() -> c_int;
    fn LIBMATTI_UI_BackendName() -> *const c_char;
    fn LIBMATTI_UI_FillRect(x: c_int, y: c_int, width: c_int, height: c_int, argb: c_uint);
    fn LIBMATTI_UI_DrawText(
        text: *const c_char,
        x: c_int,
        y: c_int,
        argb: c_uint,
        scale: c_int,
        shadow: c_int,
    );
}

fn string_from(pointer: *const c_char) -> String {
    if pointer.is_null() {
        return String::new();
    }
    unsafe { CStr::from_ptr(pointer).to_string_lossy().into_owned() }
}

pub fn window_open(width: i32, height: i32, fullscreen: bool, selection: &str, title: &str) -> bool {
    let selection = CString::new(selection).unwrap_or_default();
    let title = CString::new(title).unwrap_or_default();
    let config = WindowConfig {
        width,
        height,
        fullscreen: if fullscreen { 1 } else { 0 },
        vsync: 1,
        selection: selection.as_ptr(),
        title: title.as_ptr(),
    };
    unsafe { LIBMATTI_UI_WindowOpen(&config) != 0 }
}

pub fn window_close() {
    unsafe { LIBMATTI_UI_WindowClose() }
}

pub fn last_error() -> String {
    string_from(unsafe { LIBMATTI_UI_LastError() })
}

pub fn poll_events() {
    unsafe { LIBMATTI_UI_PollEvents() }
}

pub fn should_close() -> bool {
    unsafe { LIBMATTI_UI_ShouldClose() != 0 }
}

pub fn present() {
    unsafe { LIBMATTI_UI_Present() }
}

pub fn time() -> f64 {
    unsafe { LIBMATTI_UI_Time() }
}

pub fn save_screenshot(path: &str) -> bool {
    let path = CString::new(path).unwrap_or_default();


    unsafe { LIBMATTI_UI_SaveScreenshot(path.as_ptr()) == 0 }
}

pub fn main_window_reset() {
    unsafe { LIBMATTI_UI_MainWindow_Reset() }
}

pub fn main_window_set_screen(screen: &str) {
    let screen = CString::new(screen).unwrap_or_default();
    unsafe { LIBMATTI_UI_MainWindow_SetScreenByName(screen.as_ptr()) }
}

pub fn main_window_update() {
    unsafe { LIBMATTI_UI_MainWindow_Update() }
}

pub fn main_window_draw(time: f64) {
    unsafe { LIBMATTI_UI_MainWindow_Draw(time) }
}

pub fn main_window_should_close() -> bool {
    unsafe { LIBMATTI_UI_MainWindow_ShouldClose() != 0 }
}

pub fn main_window_take_restart() -> Option<(String, bool)> {
    let mut selection = [0i8; 64];
    let mut fullscreen: c_int = 0;
    let taken = unsafe {
        LIBMATTI_UI_MainWindow_TakeRestart(selection.as_mut_ptr(), 64, &mut fullscreen) != 0
    };
    if !taken {
        return None;
    }
    Some((string_from(selection.as_ptr()), fullscreen != 0))
}

#[allow(dead_code)]
pub fn window_width() -> i32 {
    unsafe { LIBMATTI_UI_WindowWidth() }
}

#[allow(dead_code)]
pub fn window_height() -> i32 {
    unsafe { LIBMATTI_UI_WindowHeight() }
}

#[allow(dead_code)]
pub fn backend_name() -> String {
    string_from(unsafe { LIBMATTI_UI_BackendName() })
}

#[allow(dead_code)]
pub fn fill_rect(x: i32, y: i32, width: i32, height: i32, argb: u32) {
    unsafe { LIBMATTI_UI_FillRect(x, y, width, height, argb) }
}

#[allow(dead_code)]
pub fn draw_text(text: &str, x: i32, y: i32, argb: u32, scale: i32, shadow: bool) {
    let text = CString::new(text).unwrap_or_default();
    unsafe {
        LIBMATTI_UI_DrawText(
            text.as_ptr(),
            x,
            y,
            argb,
            scale,
            if shadow { 1 } else { 0 },
        )
    }
}
