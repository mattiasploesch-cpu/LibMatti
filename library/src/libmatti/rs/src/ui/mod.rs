//! Rust half of the client: the actual window.
//!
//! Every screen, the panorama, the widget sheet, the logo, the text and the
//! texturing are C (`libmatti/matti/ui/menu.h` and `texture.h`). Rust opens the
//! game window after the early window, pumps events, forwards input to the C
//! menu state and presents the frame.

pub mod ffi;

/// The launch state the C main passes through `argv`/environment.
#[derive(Clone)]
pub struct LaunchOptions {
/// Layer of the renderer selection (`native`, `sdl`); empty means auto.
    pub layer: String,
/// API of the renderer selection (`opengl`, `vulkan`, ...); empty means auto.
    pub api: String,
    pub width: i32,
    pub height: i32,
    pub fullscreen: bool,
/// Draw exactly one frame and return (verification/headless runs).
    pub once: bool,
/// Screen to start on: main, options, pause or world (QA runs).
    pub screen: String,
}

/// `auto` and empty both mean "let the platform decide".
fn normalize(value: &str) -> String {
    let value = value.trim();
    if value.is_empty() || value.eq_ignore_ascii_case("auto") {
        String::new()
    } else {
        value.to_ascii_lowercase()
    }
}

/// Split `layer:api` (or a single legacy backend name) from the environment.
fn environment_selection() -> (String, String) {
    let mut layer = normalize(&std::env::var("MATTI_LAYER").unwrap_or_default());
    let mut api = normalize(&std::env::var("MATTI_API").unwrap_or_default());

    if layer.is_empty() && api.is_empty() {
        if let Ok(selection) = std::env::var("MATTI_RENDERER") {
            match selection.split_once(':') {
                Some((value_layer, value_api)) => {
                    layer = normalize(value_layer);
                    api = normalize(value_api);
                }
                None => match selection.as_str() {
                    "native" | "sdl" => layer = normalize(&selection),
                    "" | "auto" => {}
                    _ => api = normalize(&selection),
                },
            }
        }
    }

    if api.is_empty() && std::env::var("LIBMATTI_VK").as_deref() == Ok("TRUE") {
        api = "vulkan".to_string();
    }
    (layer, api)
}

impl LaunchOptions {
/// Accepts a full `layer:api` string or a legacy single backend name.
    fn set_selection(&mut self, selection: &str) {
        let selection = selection.trim();
        if selection.is_empty() || selection.eq_ignore_ascii_case("auto") {
            self.layer.clear();
            self.api.clear();
            return;
        }
        match selection.split_once(':') {
            Some((layer, api)) => {
                self.layer = normalize(layer);
                self.api = normalize(api);
            }
            None => match selection {
                "native" | "sdl" => {
                    self.layer = normalize(selection);
                    self.api.clear();
                }
                _ => {
                    self.layer.clear();
                    self.api = normalize(selection);
                }
            },
        }
    }

/// The string handed to C. Empty leaves the choice to `MATTI_RENDERER`
    /// (set by the vulkanClient target) and then to the platform default.
    fn selection(&self) -> String {
        if self.layer.is_empty() && self.api.is_empty() {
            return String::new();
        }
        format!(
            "{}:{}",
            if self.layer.is_empty() { "auto" } else { &self.layer },
            if self.api.is_empty() { "auto" } else { &self.api }
        )
    }

    pub fn parse(args: &[String]) -> Self {
        let (layer, api) = environment_selection();

        let mut options = LaunchOptions {
            layer,
            api,
            width: 854,
            height: 480,
            fullscreen: false,
            once: false,
            screen: "main".to_string(),
        };

        let mut index = 1;
        while index < args.len() {
            let argument = args[index].clone();
            let (name, inline) = match argument.split_once('=') {
                Some((name, value)) => (name.to_string(), Some(value.to_string())),
                None => (argument.clone(), None),
            };




            let take_value = |inline: Option<String>, index: &mut usize| -> Option<String> {
                inline.or_else(|| {
                    if *index + 1 < args.len() {
                        *index += 1;
                        Some(args[*index].clone())
                    } else {
                        None
                    }
                })
            };

            match name.as_str() {
                "--width" => {
                    if let Some(width) = take_value(inline, &mut index) {
                        options.width = width.parse().unwrap_or(options.width);
                    }
                }
                "--height" => {
                    if let Some(height) = take_value(inline, &mut index) {
                        options.height = height.parse().unwrap_or(options.height);
                    }
                }
                "--ui-screen" => {
                    if let Some(screen) = take_value(inline, &mut index) {
                        options.screen = screen;
                    }
                }
                "--layer" => {
                    if let Some(value) = take_value(inline, &mut index) {
                        options.layer = normalize(&value);
                    }
                }
                "--api" => {
                    if let Some(value) = take_value(inline, &mut index) {
                        options.api = normalize(&value);
                    }
                }
                "--fullscreen" => options.fullscreen = true,
                "--ui-once" => options.once = true,
                _ => {}
            }

            index += 1;
        }

        options
    }
}

pub fn run(args: &[String]) -> i32 {
    let mut options = LaunchOptions::parse(args);

    loop {
        let selection = options.selection();
        if !ffi::window_open(
            options.width,
            options.height,
            options.fullscreen,
            &selection,
            "Matticraft",
        ) {
            eprintln!(
                "matti-rs: could not open the game window: {}",
                ffi::last_error()
            );
            return 1;
        }

        ffi::main_window_reset();
        ffi::main_window_set_screen(&options.screen);

        loop {
            if ffi::should_close() {
                break;
            }
            ffi::poll_events();
            ffi::main_window_update();
            if ffi::main_window_should_close() {
                break;
            }

            ffi::main_window_draw(ffi::time());
            ffi::present();

            if options.once {
                if let Ok(path) = std::env::var("MATTI_UI_SCREENSHOT") {
                    if !path.is_empty() && !ffi::save_screenshot(&path) {
                        eprintln!("matti-rs: could not write the screenshot to {path}");
                    }
                }
                break;
            }
        }

        ffi::window_close();

        match ffi::main_window_take_restart() {
            Some((selection, fullscreen)) => {
                options.set_selection(&selection);
                options.fullscreen = fullscreen;
            }
            None => return 0,
        }
    }
}
