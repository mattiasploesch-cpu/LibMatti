use eframe::egui;
use egui::{Color32, FontFamily, FontId, Stroke, TextStyle};

pub const SMALL_CAPS: &str = "smallcaps";

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ThemePreset {
    Matticraft,
    NoRisk,
    Midnight,
    Ocean,
    Ember,
    Mono,
}

impl ThemePreset {
    pub const ALL: [ThemePreset; 6] = [
        ThemePreset::Matticraft,
        ThemePreset::NoRisk,
        ThemePreset::Midnight,
        ThemePreset::Ocean,
        ThemePreset::Ember,
        ThemePreset::Mono,
    ];

    pub fn label(self) -> &'static str {
        match self {
            ThemePreset::Matticraft => "Matticraft",
            ThemePreset::NoRisk => "NoRisk",
            ThemePreset::Midnight => "Midnight",
            ThemePreset::Ocean => "Ocean",
            ThemePreset::Ember => "Ember",
            ThemePreset::Mono => "Mono",
        }
    }

    pub fn description(self) -> &'static str {
        match self {
            ThemePreset::Matticraft => "The green Material 3 palette from the Android client.",
            ThemePreset::NoRisk => "Neutral chrome with the original cobalt signal.",
            ThemePreset::Midnight => "Deep blue-black with electric violet.",
            ThemePreset::Ocean => "Cold teal glass and a bright cyan edge.",
            ThemePreset::Ember => "Warm charcoal with an amber highlight.",
            ThemePreset::Mono => "Paper-white controls on graphite.",
        }
    }

    pub fn primary(self) -> Color32 {
        match self {
            ThemePreset::Matticraft => Color32::from_rgb(155, 225, 93),
            ThemePreset::NoRisk => Color32::from_rgb(79, 142, 255),
            ThemePreset::Midnight => Color32::from_rgb(157, 120, 255),
            ThemePreset::Ocean => Color32::from_rgb(64, 214, 208),
            ThemePreset::Ember => Color32::from_rgb(245, 158, 64),
            ThemePreset::Mono => Color32::from_rgb(235, 235, 235),
        }
    }

    pub fn primary_container(self) -> Color32 {
        match self {
            ThemePreset::Matticraft => Color32::from_rgb(23, 48, 15),
            ThemePreset::NoRisk => Color32::from_rgb(30, 74, 138),
            ThemePreset::Midnight => Color32::from_rgb(61, 43, 122),
            ThemePreset::Ocean => Color32::from_rgb(15, 88, 91),
            ThemePreset::Ember => Color32::from_rgb(112, 61, 19),
            ThemePreset::Mono => Color32::from_rgb(73, 73, 73),
        }
    }

    pub fn next(self) -> ThemePreset {
        let index = ThemePreset::ALL
            .iter()
            .position(|preset| *preset == self)
            .unwrap_or(0);
        ThemePreset::ALL[(index + 1) % ThemePreset::ALL.len()]
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Theme {
    pub preset: ThemePreset,
    pub primary: Color32,
    pub on_primary: Color32,
    pub primary_container: Color32,
    pub on_primary_container: Color32,
    pub secondary: Color32,
    pub on_secondary: Color32,
    pub bg: Color32,
    pub surface: Color32,
    pub surface_variant: Color32,
    pub on_surface: Color32,
    pub on_surface_variant: Color32,
    pub outline: Color32,
    pub outline_bright: Color32,
    pub error: Color32,
    pub on_error: Color32,
    pub error_container: Color32,
    pub on_error_container: Color32,
    pub success: Color32,
    pub warning: Color32,
    pub shadow: Color32,
    pub overlay: Color32,
    pub accent: Color32,
    pub accent_dark: Color32,
    pub panel: Color32,
    pub card: Color32,
    pub card_hover: Color32,
    pub nav: Color32,
    pub border: Color32,
    pub border_bright: Color32,
    pub text: Color32,
    pub muted: Color32,
    pub faint: Color32,
    pub danger: Color32,
    pub info: Color32,
}

impl Theme {
    pub fn matticraft() -> Self {
        Theme::new(ThemePreset::Matticraft)
    }

    pub fn new(preset: ThemePreset) -> Self {
        let primary = preset.primary();
        let primary_container = preset.primary_container();
        let (bg, surface, surface_variant, ink, muted, outline, secondary) = match preset {
            ThemePreset::Matticraft => (
                Color32::from_rgb(12, 17, 13),
                Color32::from_rgb(23, 31, 24),
                Color32::from_rgb(34, 48, 38),
                Color32::from_rgb(232, 242, 228),
                Color32::from_rgb(169, 187, 170),
                Color32::from_rgb(76, 101, 82),
                Color32::from_rgb(127, 183, 255),
            ),
            ThemePreset::NoRisk => (
                Color32::from_rgb(13, 15, 18),
                Color32::from_rgb(23, 25, 30),
                Color32::from_rgb(31, 34, 40),
                Color32::from_rgb(241, 244, 249),
                Color32::from_rgb(164, 171, 184),
                Color32::from_rgb(49, 53, 61),
                Color32::from_rgb(127, 183, 255),
            ),
            ThemePreset::Midnight => (
                Color32::from_rgb(11, 12, 22),
                Color32::from_rgb(20, 21, 38),
                Color32::from_rgb(31, 32, 54),
                Color32::from_rgb(239, 235, 255),
                Color32::from_rgb(179, 174, 199),
                Color32::from_rgb(63, 58, 91),
                Color32::from_rgb(127, 183, 255),
            ),
            ThemePreset::Ocean => (
                Color32::from_rgb(7, 19, 22),
                Color32::from_rgb(15, 33, 37),
                Color32::from_rgb(23, 49, 54),
                Color32::from_rgb(224, 249, 248),
                Color32::from_rgb(157, 192, 191),
                Color32::from_rgb(47, 91, 94),
                Color32::from_rgb(127, 183, 255),
            ),
            ThemePreset::Ember => (
                Color32::from_rgb(21, 15, 11),
                Color32::from_rgb(35, 26, 19),
                Color32::from_rgb(52, 38, 27),
                Color32::from_rgb(255, 239, 225),
                Color32::from_rgb(196, 171, 151),
                Color32::from_rgb(104, 76, 54),
                Color32::from_rgb(127, 183, 255),
            ),
            ThemePreset::Mono => (
                Color32::from_rgb(8, 8, 8),
                Color32::from_rgb(18, 18, 18),
                Color32::from_rgb(31, 31, 31),
                Color32::from_rgb(242, 242, 242),
                Color32::from_rgb(166, 166, 166),
                Color32::from_rgb(74, 74, 74),
                Color32::from_rgb(196, 196, 196),
            ),
        };
        Theme {
            preset,
            primary,
            on_primary: if preset == ThemePreset::Matticraft {
                Color32::from_rgb(23, 48, 15)
            } else {
                Color32::from_rgb(12, 20, 12)
            },
            primary_container,
            on_primary_container: if preset == ThemePreset::Matticraft {
                primary
            } else {
                Color32::from_rgb(238, 238, 238)
            },
            secondary,
            on_secondary: Color32::from_rgb(6, 23, 45),
            bg,
            surface,
            surface_variant,
            on_surface: ink,
            on_surface_variant: muted,
            outline,
            outline_bright: Color32::from_rgb(
                outline.r().saturating_add(30),
                outline.g().saturating_add(30),
                outline.b().saturating_add(30),
            ),
            error: Color32::from_rgb(255, 180, 171),
            on_error: Color32::from_rgb(105, 0, 5),
            error_container: Color32::from_rgb(147, 0, 10),
            on_error_container: Color32::from_rgb(255, 218, 214),
            success: Color32::from_rgb(155, 225, 93),
            warning: Color32::from_rgb(245, 190, 94),
            shadow: Color32::from_black_alpha(150),
            overlay: Color32::from_black_alpha(184),
            accent: primary,
            accent_dark: primary_container,
            panel: surface,
            card: if preset == ThemePreset::Matticraft {
                Color32::from_rgb(26, 36, 28)
            } else {
                surface_variant
            },
            card_hover: surface_variant,
            nav: bg,
            border: outline,
            border_bright: Color32::from_rgb(
                outline.r().saturating_add(34),
                outline.g().saturating_add(34),
                outline.b().saturating_add(34),
            ),
            text: ink,
            muted,
            faint: Color32::from_rgb(110, 124, 112),
            danger: Color32::from_rgb(255, 180, 171),
            info: secondary,
        }
    }

    pub fn with_primary(mut self, primary: Color32) -> Self {
        self.primary = primary;
        self.accent = primary;
        self.success = primary;
        self
    }

    pub fn alpha(color: Color32, alpha: u8) -> Color32 {
        Color32::from_rgba_unmultiplied(color.r(), color.g(), color.b(), alpha)
    }

    pub fn display_font(size: f32) -> FontId {
        FontId::new(size, FontFamily::Name(SMALL_CAPS.into()))
    }

    pub fn heading(&self) -> FontId {
        FontId::proportional(28.0)
    }

    pub fn subheading(&self) -> FontId {
        FontId::proportional(18.0)
    }

    pub fn body(&self) -> FontId {
        FontId::proportional(13.5)
    }

    pub fn small(&self) -> FontId {
        FontId::proportional(11.0)
    }

    pub fn micro(&self) -> FontId {
        FontId::proportional(9.5)
    }

    pub fn mono(&self) -> FontId {
        FontId::monospace(11.0)
    }

    pub fn apply(&self, ctx: &egui::Context) {
        let mut visuals = egui::Visuals::dark();
        visuals.panel_fill = self.bg;
        visuals.window_fill = self.surface;
        visuals.extreme_bg_color = Color32::from_rgb(7, 10, 8);
        visuals.faint_bg_color = self.surface_variant;
        visuals.code_bg_color = Color32::from_rgb(9, 13, 10);
        visuals.selection.bg_fill = self.primary_container;
        visuals.selection.stroke = Stroke::new(1.0, self.primary);
        visuals.hyperlink_color = self.secondary;
        visuals.warn_fg_color = self.warning;
        visuals.error_fg_color = self.error;
        visuals.widgets.inactive.bg_fill = self.surface_variant;
        visuals.widgets.inactive.weak_bg_fill = self.surface;
        visuals.widgets.inactive.bg_stroke = Stroke::new(1.0, self.outline);
        visuals.widgets.inactive.fg_stroke = Stroke::new(1.0, self.on_surface_variant);
        visuals.widgets.hovered.bg_fill = self.card_hover;
        visuals.widgets.hovered.weak_bg_fill = self.surface_variant;
        visuals.widgets.hovered.bg_stroke = Stroke::new(1.0, self.primary);
        visuals.widgets.hovered.fg_stroke = Stroke::new(1.0, self.on_surface);
        visuals.widgets.active.bg_fill = self.primary_container;
        visuals.widgets.active.weak_bg_fill = self.primary_container;
        visuals.widgets.active.bg_stroke = Stroke::new(1.0, self.primary);
        visuals.widgets.active.fg_stroke = Stroke::new(1.0, self.on_surface);
        visuals.widgets.noninteractive.bg_fill = self.surface;
        visuals.widgets.noninteractive.bg_stroke = Stroke::new(1.0, self.outline);
        visuals.widgets.noninteractive.fg_stroke = Stroke::new(1.0, self.on_surface);
        visuals.window_stroke = Stroke::new(1.0, self.outline);
        visuals.window_corner_radius = egui::CornerRadius::same(16);
        visuals.menu_corner_radius = egui::CornerRadius::same(12);
        visuals.popup_shadow = egui::epaint::Shadow {
            offset: [0, 10],
            blur: 28,
            spread: 0,
            color: self.shadow,
        };
        visuals.window_shadow = visuals.popup_shadow;
        ctx.set_visuals(visuals);

        let mut style = (*ctx.style()).clone();
        style.spacing.item_spacing = egui::vec2(8.0, 8.0);
        style.spacing.button_padding = egui::vec2(14.0, 8.0);
        style.spacing.interact_size = egui::vec2(38.0, 34.0);
        style.spacing.slider_width = 170.0;
        style.spacing.combo_width = 190.0;
        style.spacing.indent = 18.0;
        style.text_styles = [
            (TextStyle::Heading, self.heading()),
            (TextStyle::Body, self.body()),
            (TextStyle::Button, FontId::proportional(12.5)),
            (TextStyle::Small, self.small()),
            (TextStyle::Monospace, self.mono()),
        ]
        .into();
        ctx.set_style(style);
    }
}

pub fn card_frame(theme: &Theme) -> egui::Frame {
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(16))
}

pub fn soft_frame(theme: &Theme) -> egui::Frame {
    egui::Frame::NONE
        .fill(theme.surface)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(14))
}

pub fn inset_frame(theme: &Theme) -> egui::Frame {
    egui::Frame::NONE
        .fill(theme.bg)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(12))
        .inner_margin(egui::Margin::symmetric(12, 8))
}

pub fn modal_frame(theme: &Theme) -> egui::Frame {
    egui::Frame::NONE
        .fill(theme.surface)
        .stroke(Stroke::new(1.0, theme.outline_bright))
        .corner_radius(egui::CornerRadius::same(20))
        .inner_margin(egui::Margin::same(22))
        .shadow(egui::epaint::Shadow {
            offset: [0, 16],
            blur: 48,
            spread: 2,
            color: theme.shadow,
        })
}

pub fn paint_grid(ui: &egui::Ui, rect: egui::Rect, theme: &Theme) {
    let painter = ui.painter();
    let color = Theme::alpha(theme.outline, 34);
    let step = 32.0;
    let mut x = rect.left();
    while x <= rect.right() {
        painter.line_segment(
            [egui::pos2(x, rect.top()), egui::pos2(x, rect.bottom())],
            Stroke::new(1.0, color),
        );
        x += step;
    }
    let mut y = rect.top();
    while y <= rect.bottom() {
        painter.line_segment(
            [egui::pos2(rect.left(), y), egui::pos2(rect.right(), y)],
            Stroke::new(1.0, color),
        );
        y += step;
    }
}

pub fn paint_glow(ui: &egui::Ui, center: egui::Pos2, radius: f32, color: Color32) {
    let painter = ui.painter();
    for index in (1..=10).rev() {
        let fraction = index as f32 / 10.0;
        painter.circle_filled(
            center,
            radius * fraction,
            Theme::alpha(color, (22.0 * (1.0 - fraction)) as u8),
        );
    }
}

pub fn accent_choices() -> [Color32; 8] {
    [
        Color32::from_rgb(155, 225, 93),
        Color32::from_rgb(127, 183, 255),
        Color32::from_rgb(79, 142, 255),
        Color32::from_rgb(157, 120, 255),
        Color32::from_rgb(64, 214, 208),
        Color32::from_rgb(245, 190, 94),
        Color32::from_rgb(244, 113, 113),
        Color32::from_rgb(235, 235, 235),
    ]
}
