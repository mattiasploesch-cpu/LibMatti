//! Platform and renderer-preset detection.
//!
//! The launcher does not hard-code a renderer. It asks the one `libmatti`
//! (`LIBMATTI_Platform_DefaultRenderer`) when the library is next to the
//! launcher or on the loader path, and only falls back to a local copy of the
//! same table when the game is not installed yet. This keeps launcher and game
//! defaults identical without linking the whole game into the launcher.

use std::path::PathBuf;
use std::sync::OnceLock;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Platform {
    Linux,
    Windows,
    MacOs,
    Android,
    FreeBsd,
    Unknown,
}

impl Platform {
    pub fn current() -> Self {
        if cfg!(target_os = "android") {
            Platform::Android
        } else if cfg!(target_os = "windows") {
            Platform::Windows
        } else if cfg!(target_os = "macos") {
            Platform::MacOs
        } else if cfg!(target_os = "freebsd") {
            Platform::FreeBsd
        } else if cfg!(target_os = "linux") {
            Platform::Linux
        } else {
            Platform::Unknown
        }
    }

    pub fn name(self) -> &'static str {
        match self {
            Platform::Linux => "linux",
            Platform::Windows => "windows",
            Platform::MacOs => "macos",
            Platform::Android => "android",
            Platform::FreeBsd => "freebsd",
            Platform::Unknown => "unknown",
        }
    }

    pub fn is_desktop(self) -> bool {
        !matches!(self, Platform::Android | Platform::Unknown)
    }

    /// The local copy of `LIBMATTI_Platform_DefaultRenderer`.
    pub fn local_default_selection(self) -> &'static str {
        match self {
            Platform::Android => {
                if bits() >= 64 {
                    "sdl:vulkan"
                } else {
                    "sdl:opengl"
                }
            }
            Platform::Windows => "sdl:directx",
            Platform::MacOs => "sdl:opengl",
            Platform::Linux | Platform::FreeBsd => {
                if bits() >= 64 {
                    "native:vulkan"
                } else {
                    "native:opengl"
                }
            }
            Platform::Unknown => "sdl:auto",
        }
    }
}

/// Archive name of the target architecture (`x86_64`, `arm64`, ...).
pub fn arch() -> &'static str {
    match std::env::consts::ARCH {
        "x86_64" => "x86_64",
        "x86" => "x86",
        "aarch64" => "arm64",
        "arm" => "arm",
        "riscv64" => "riscv64",
        other => other,
    }
}

pub fn bits() -> u32 {
    usize::BITS
}

/// The renderer preset for this host. Prefer the game library, then the
/// explicit environment override used by portable packages, then the table.
pub fn default_selection() -> String {
    if let Some(selection) = query_libmatti_default() {
        return selection;
    }
    if let Ok(selection) = std::env::var("MATTI_DEFAULT_SELECTION") {
        if crate::profile::parse_selection(&selection).is_ok() {
            return selection;
        }
    }
    Platform::current().local_default_selection().to_string()
}

/// Where the preset came from (`libmatti`, `environment` or `builtin`).
pub fn default_selection_source() -> &'static str {
    if query_libmatti_default().is_some() {
        "libmatti"
    } else if std::env::var("MATTI_DEFAULT_SELECTION")
        .ok()
        .is_some_and(|value| crate::profile::parse_selection(&value).is_ok())
    {
        "environment"
    } else {
        "builtin"
    }
}

fn library_names() -> &'static [&'static str] {
    if cfg!(target_os = "windows") {
        &["matti.dll"]
    } else if cfg!(target_os = "macos") {
        &["libmatti.dylib"]
    } else {
        &["libmatti.so.0", "libmatti.so"]
    }
}

fn library_candidates() -> Vec<PathBuf> {
    let mut candidates = Vec::new();
    if let Ok(explicit) = std::env::var("MATTI_LIBMATTI") {
        if !explicit.is_empty() {
            candidates.push(PathBuf::from(explicit));
        }
    }
    if let Ok(exe) = std::env::current_exe() {
        if let Some(dir) = exe.parent() {
            for name in library_names() {
                candidates.push(dir.join(name));
                candidates.push(dir.join("lib").join(name));
                candidates.push(dir.join("..").join("lib").join(name));
            }
            // macOS app bundle: Contents/MacOS/<binary> + Contents/Frameworks.
            if cfg!(target_os = "macos") {
                for name in library_names() {
                    candidates.push(dir.join("..").join("Frameworks").join(name));
                }
            }
        }
    }
    for name in library_names() {
        // An empty parent makes the platform loader search its normal paths.
        candidates.push(PathBuf::from(name));
    }
    candidates
}

fn query_libmatti_default() -> Option<String> {
    static CACHE: OnceLock<Option<String>> = OnceLock::new();
    CACHE
        .get_or_init(|| {
            for candidate in library_candidates() {
                if candidate.is_absolute() && !candidate.exists() {
                    continue;
                }
                if let Some(value) = unsafe { read_default_renderer(&candidate) } {
                    if crate::profile::parse_selection(&value).is_ok() {
                        return Some(value);
                    }
                }
            }
            None
        })
        .clone()
}

#[cfg(unix)]
mod dynamic {
    use std::ffi::{c_char, c_int, c_void, CString};
    use std::path::Path;

    #[cfg_attr(not(target_os = "macos"), link(name = "dl"))]
    extern "C" {
        fn dlopen(filename: *const c_char, flag: c_int) -> *mut c_void;
        fn dlsym(handle: *mut c_void, symbol: *const c_char) -> *mut c_void;
    }

    const RTLD_NOW: c_int = 2;

    pub unsafe fn open(path: &Path) -> Option<*mut c_void> {
        let path = CString::new(path.to_string_lossy().as_bytes()).ok()?;
        let handle = dlopen(path.as_ptr(), RTLD_NOW);
        (!handle.is_null()).then_some(handle)
    }

    pub unsafe fn symbol(handle: *mut c_void, name: &str) -> Option<*mut c_void> {
        let name = CString::new(name).ok()?;
        let symbol = dlsym(handle, name.as_ptr());
        (!symbol.is_null()).then_some(symbol)
    }
}

#[cfg(windows)]
mod dynamic {
    use std::ffi::{c_void, CString};
    use std::path::Path;

    #[link(name = "kernel32")]
    extern "system" {
        fn LoadLibraryA(name: *const u8) -> *mut c_void;
        fn GetProcAddress(module: *mut c_void, name: *const u8) -> *mut c_void;
    }

    pub unsafe fn open(path: &Path) -> Option<*mut c_void> {
        let path = CString::new(path.to_string_lossy().as_bytes()).ok()?;
        let handle = LoadLibraryA(path.as_ptr() as *const u8);
        (!handle.is_null()).then_some(handle)
    }

    pub unsafe fn symbol(handle: *mut c_void, name: &str) -> Option<*mut c_void> {
        let name = CString::new(name).ok()?;
        let symbol = GetProcAddress(handle, name.as_ptr() as *const u8);
        (!symbol.is_null()).then_some(symbol)
    }
}

unsafe fn read_default_renderer(path: &std::path::Path) -> Option<String> {
    use std::os::raw::c_char;
    type DefaultRendererFn = unsafe extern "C" fn() -> *const c_char;

    let handle = dynamic::open(path)?;
    // The handle is intentionally not closed: the function pointer and the
    // CString it returns stay valid for the rest of the process.
    let symbol = dynamic::symbol(handle, "LIBMATTI_Platform_DefaultRenderer")?;
    let function: DefaultRendererFn = std::mem::transmute(symbol);
    let value = function();
    if value.is_null() {
        return None;
    }
    std::ffi::CStr::from_ptr(value)
        .to_str()
        .ok()
        .map(ToString::to_string)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn arch_and_bits_are_sane() {
        assert!(!arch().is_empty());
        assert!(bits() == 32 || bits() == 64);
    }

    #[test]
    fn local_defaults_match_platform_policy() {
        assert_eq!(Platform::Windows.local_default_selection(), "sdl:directx");
        assert_eq!(Platform::MacOs.local_default_selection(), "sdl:opengl");
        if bits() >= 64 {
            assert_eq!(
                Platform::Linux.local_default_selection(),
                "native:vulkan"
            );
        } else {
            assert_eq!(Platform::Linux.local_default_selection(), "native:opengl");
        }
    }

    #[test]
    fn default_selection_is_always_parseable() {
        let selection = default_selection();
        assert!(crate::profile::parse_selection(&selection).is_ok());
    }
}
