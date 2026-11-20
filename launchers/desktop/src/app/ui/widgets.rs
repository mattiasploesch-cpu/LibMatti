use eframe::egui;
use egui::{
    Align, Align2, Color32, FontId, Layout, Rect, Response, RichText, Sense, Stroke, StrokeKind, Ui,
    Vec2,
};

use super::icons::{self, Icon};
use super::theme::Theme;

pub fn page_header(
    ui: &mut Ui,
    theme: &Theme,
    eyebrow: &str,
    title: &str,
    subtitle: &str,
    actions: impl FnOnce(&mut Ui),
) {
    ui.horizontal(|ui| {
        ui.vertical(|ui| {
            label(ui, theme, eyebrow, theme.primary, 9.5, true);
            ui.add_space(1.0);
            ui.label(
                RichText::new(title)
                    .font(FontId::proportional(28.0))
                    .strong()
                    .color(theme.on_surface),
            );
            ui.label(
                RichText::new(subtitle)
                    .font(theme.body())
                    .color(theme.on_surface_variant),
            );
        });
        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
            actions(ui);
        });
    });
}

pub fn label(ui: &mut Ui, theme: &Theme, text: &str, color: Color32, size: f32, strong: bool) {
    let mut text = RichText::new(text)
        .font(FontId::proportional(size))
        .color(color);
    if strong {
        text = text.strong();
    }
    ui.label(text);
}

pub fn section_title(ui: &mut Ui, theme: &Theme, title: &str, hint: &str) {
    ui.horizontal(|ui| {
        ui.label(
            RichText::new(title)
                .font(theme.subheading())
                .strong()
                .color(theme.on_surface),
        );
        if !hint.is_empty() {
            ui.label(
                RichText::new(hint)
                    .font(theme.small())
                    .color(theme.on_surface_variant),
            );
        }
    });
}

pub fn primary_button(ui: &mut Ui, theme: &Theme, text: &str, icon: Option<Icon>) -> Response {
    let button = egui::Button::new(
        RichText::new(format!("{}  {}", icon.map(icon_glyph).unwrap_or(""), text))
            .font(FontId::proportional(14.0))
            .strong()
            .color(theme.on_primary),
    )
    .fill(theme.primary)
    .stroke(Stroke::NONE)
    .corner_radius(egui::CornerRadius::same(14));
    ui.add_sized([ui.available_width().min(220.0), 42.0], button)
}

pub fn secondary_button(ui: &mut Ui, theme: &Theme, text: &str, icon: Option<Icon>) -> Response {
    let button = egui::Button::new(
        RichText::new(format!("{}  {}", icon.map(icon_glyph).unwrap_or(""), text))
            .font(FontId::proportional(12.5))
            .color(theme.on_surface),
    )
    .fill(theme.surface_variant)
    .stroke(Stroke::new(1.0, theme.outline))
    .corner_radius(egui::CornerRadius::same(12));
    ui.add_sized([ui.available_width().min(190.0), 38.0], button)
}

pub fn text_button(ui: &mut Ui, theme: &Theme, text: &str, icon: Option<Icon>) -> Response {
    let response = ui.add_sized(
        [ui.available_width().min(170.0), 32.0],
        egui::Button::new(
            RichText::new(format!("{}  {}", icon.map(icon_glyph).unwrap_or(""), text))
                .font(theme.small())
                .color(theme.primary),
        )
        .fill(Color32::TRANSPARENT)
        .stroke(Stroke::NONE),
    );
    response
}

pub fn danger_button(ui: &mut Ui, theme: &Theme, text: &str) -> Response {
    ui.add_sized(
        [ui.available_width().min(170.0), 38.0],
        egui::Button::new(
            RichText::new(text)
                .font(FontId::proportional(12.5))
                .color(theme.on_error),
        )
        .fill(theme.error_container)
        .stroke(Stroke::new(1.0, theme.error))
        .corner_radius(egui::CornerRadius::same(12)),
    )
}

pub fn chip(ui: &mut Ui, theme: &Theme, text: &str, active: bool) -> Response {
    let fill = if active { theme.primary_container } else { theme.surface_variant };
    let color = if active { theme.primary } else { theme.on_surface_variant };
    let stroke = if active { theme.primary } else { theme.outline };
    ui.add(
        egui::Button::new(RichText::new(text).font(theme.small()).color(color).strong())
            .fill(fill)
            .stroke(Stroke::new(1.0, stroke))
            .corner_radius(egui::CornerRadius::same(20))
            .min_size(Vec2::new(0.0, 28.0)),
    )
}

pub fn badge(ui: &mut Ui, theme: &Theme, text: &str, color: Color32) {
    let text = RichText::new(text)
        .font(theme.micro())
        .color(color)
        .strong();
    let galley = ui.painter().layout_no_wrap(text.text().to_owned(), theme.micro(), color);
    let size = Vec2::new(galley.size().x + 14.0, 20.0);
    let (rect, _) = ui.allocate_exact_size(size, Sense::hover());
    ui.painter().rect_filled(rect, 10.0, Theme::alpha(color, 28));
    ui.painter().text(
        rect.center(),
        Align2::CENTER_CENTER,
        text.text(),
        theme.micro(),
        color,
    );
}

pub fn progress(ui: &mut Ui, theme: &Theme, value: f32, color: Color32) {
    let (rect, _) = ui.allocate_exact_size(Vec2::new(ui.available_width(), 6.0), Sense::hover());
    ui.painter()
        .rect_filled(rect, 3.0, Theme::alpha(theme.on_surface_variant, 34));
    let width = (rect.width() * value.clamp(0.0, 1.0)).max(2.0);
    ui.painter().rect_filled(
        Rect::from_min_size(rect.min, Vec2::new(width, rect.height())),
        3.0,
        color,
    );
}

pub fn avatar(ui: &mut Ui, theme: &Theme, name: &str, size: f32) {
    let (rect, _) = ui.allocate_exact_size(Vec2::splat(size), Sense::hover());
    ui.painter().circle_filled(rect.center(), size * 0.5, theme.primary_container);
    ui.painter()
        .circle_stroke(rect.center(), size * 0.5, Stroke::new(1.0, theme.primary));
    let initial = name.chars().next().unwrap_or('?').to_uppercase().to_string();
    ui.painter().text(
        rect.center(),
        Align2::CENTER_CENTER,
        initial,
        FontId::proportional(size * 0.42),
        theme.primary,
    );
}

pub fn stat(ui: &mut Ui, theme: &Theme, value: &str, label: &str, icon: Icon) {
    egui::Frame::NONE
        .fill(theme.surface_variant)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(14))
        .inner_margin(egui::Margin::same(12))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                icons::paint(
                    ui.painter(),
                    Rect::from_min_size(ui.cursor().min, Vec2::splat(18.0)),
                    icon,
                    theme.primary,
                );
                ui.add_space(26.0);
                ui.vertical(|ui| {
                    ui.label(
                        RichText::new(value)
                            .font(FontId::proportional(18.0))
                            .strong()
                            .color(theme.on_surface),
                    );
                    ui.label(
                        RichText::new(label)
                            .font(theme.small())
                            .color(theme.on_surface_variant),
                    );
                });
            });
        });
}

pub fn empty_state(ui: &mut Ui, theme: &Theme, icon: Icon, title: &str, detail: &str) {
    ui.vertical_centered(|ui| {
        ui.add_space(34.0);
        let size = 48.0;
        let (rect, _) = ui.allocate_exact_size(Vec2::splat(size), Sense::hover());
        ui.painter()
            .circle_filled(rect.center(), size * 0.5, theme.surface_variant);
        icons::paint(ui.painter(), rect.shrink(13.0), icon, theme.on_surface_variant);
        ui.add_space(12.0);
        ui.label(
            RichText::new(title)
                .font(theme.subheading())
                .strong()
                .color(theme.on_surface),
        );
        ui.label(
            RichText::new(detail)
                .font(theme.small())
                .color(theme.on_surface_variant),
        );
        ui.add_space(20.0);
    });
}

pub fn key_value(ui: &mut Ui, theme: &Theme, key: &str, value: &str) {
    ui.horizontal(|ui| {
        ui.label(
            RichText::new(key)
                .font(theme.small())
                .color(theme.on_surface_variant),
        );
        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
            ui.label(
                RichText::new(value)
                    .font(theme.small())
                    .color(theme.on_surface),
            );
        });
    });
}

pub fn search_field(ui: &mut Ui, theme: &Theme, value: &mut String, hint: &str) -> Response {
    let response = egui::Frame::NONE
        .fill(theme.bg)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(12))
        .inner_margin(egui::Margin::symmetric(10, 7))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                icons::paint(
                    ui.painter(),
                    Rect::from_min_size(ui.cursor().min, Vec2::splat(16.0)),
                    Icon::Search,
                    theme.on_surface_variant,
                );
                ui.add_space(22.0);
                ui.add(
                    egui::TextEdit::singleline(value)
                        .hint_text(hint)
                        .frame(false)
                        .desired_width(ui.available_width()),
                )
            })
            .inner
        })
        .inner;
    response
}

pub fn toggle_row(ui: &mut Ui, theme: &Theme, title: &str, detail: &str, value: &mut bool) -> bool {
    let mut changed = false;
    ui.horizontal(|ui| {
        ui.vertical(|ui| {
            ui.label(
                RichText::new(title)
                    .font(theme.body())
                    .color(theme.on_surface),
            );
            ui.label(
                RichText::new(detail)
                    .font(theme.small())
                    .color(theme.on_surface_variant),
            );
        });
        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
            changed = ui.add(egui::Checkbox::new(value, "")).changed();
        });
    });
    changed
}

pub fn segmented<T: Copy + PartialEq>(
    ui: &mut Ui,
    theme: &Theme,
    values: &[T],
    selected: &mut T,
    label: impl Fn(T) -> String,
) -> bool {
    let mut changed = false;
    ui.horizontal_wrapped(|ui| {
        for value in values {
            let active = *selected == *value;
            if chip(ui, theme, &label(*value), active).clicked() {
                *selected = *value;
                changed = true;
            }
        }
    });
    changed
}

pub fn ping_bars(ui: &mut Ui, theme: &Theme, ping: u32) {
    let height = 15.0;
    let width = 3.0;
    for index in 0..5 {
        let active = index as u32 * 40 <= ping || ping == 0;
        let color = if active {
            if index < 2 {
                theme.success
            } else if index < 4 {
                theme.warning
            } else {
                theme.error
            }
        } else {
            Theme::alpha(theme.on_surface_variant, 50)
        };
        let (rect, _) = ui.allocate_exact_size(Vec2::new(width, height), Sense::hover());
        let bar = Rect::from_min_size(
            egui::pos2(rect.left(), rect.bottom() - 3.0 - index as f32 * 2.6),
            Vec2::new(width, 3.0 + index as f32 * 2.6),
        );
        ui.painter().rect_filled(bar, 1.0, color);
        ui.add_space(1.0);
    }
}

pub fn tag(ui: &mut Ui, theme: &Theme, text: &str) {
    let response = ui.label(
        RichText::new(text)
            .font(theme.micro())
            .color(theme.secondary)
            .strong(),
    );
    let rect = response.rect.expand2(Vec2::new(7.0, 3.0));
    ui.painter()
        .rect_filled(rect, 9.0, Theme::alpha(theme.secondary, 24));
}

pub fn divider(ui: &mut Ui, theme: &Theme) {
    let (rect, _) = ui.allocate_exact_size(Vec2::new(ui.available_width(), 1.0), Sense::hover());
    ui.painter()
        .rect_filled(rect, 0.0, Theme::alpha(theme.outline, 150));
}

pub fn skin_preview(ui: &mut Ui, texture: Option<&egui::TextureHandle>, theme: &Theme, size: f32) {
    let (rect, _) = ui.allocate_exact_size(Vec2::splat(size), Sense::hover());
    ui.painter()
        .rect_filled(rect, 16.0, theme.surface_variant);
    if let Some(texture) = texture {
        let uv = Rect::from_min_max(egui::pos2(0.0, 0.0), egui::pos2(1.0, 1.0));
        ui.painter().image(texture.id(), rect.shrink(8.0), uv, Color32::WHITE);
    } else {
        icons::paint(ui.painter(), rect.shrink(20.0), Icon::User, theme.primary);
    }
}

pub fn hero_panel(ui: &mut Ui, theme: &Theme, add: impl FnOnce(&mut Ui)) {
    egui::Frame::NONE
        .fill(theme.surface)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(20))
        .inner_margin(egui::Margin::same(22))
        .show(ui, |ui| {
            add(ui);
        });
}

pub fn list_row(ui: &mut Ui, theme: &Theme, selected: bool, add: impl FnOnce(&mut Ui)) -> Response {
    let response = egui::Frame::NONE
        .fill(if selected { theme.primary_container } else { theme.card })
        .stroke(Stroke::new(
            1.0,
            if selected { theme.primary } else { theme.outline },
        ))
        .corner_radius(egui::CornerRadius::same(14))
        .inner_margin(egui::Margin::symmetric(14, 12))
        .show(ui, |ui| {
            add(ui);
        })
        .response;
    response.interact(Sense::click())
}

pub fn icon_glyph(icon: Icon) -> &'static str {
    match icon {
        Icon::Home => "⌂",
        Icon::Play => "▶",
        Icon::Grid => "▦",
        Icon::Layers => "▤",
        Icon::User => "●",
        Icon::Users => "●●",
        Icon::Globe => "◎",
        Icon::Sparkles => "✦",
        Icon::News => "▥",
        Icon::Download => "↓",
        Icon::Terminal => ">_",
        Icon::Settings => "⚙",
        Icon::Info => "i",
        Icon::Search => "⌕",
        Icon::Bell => "●",
        Icon::Plus => "+",
        Icon::Minus => "−",
        Icon::Close => "×",
        Icon::Check => "✓",
        Icon::Warning => "!",
        Icon::Refresh => "↻",
        Icon::Folder => "▱",
        Icon::Trash => "×",
        Icon::Copy => "⧉",
        Icon::External => "↗",
        Icon::ChevronRight => "›",
        Icon::ChevronDown => "⌄",
        Icon::ArrowLeft => "←",
        Icon::ArrowRight => "→",
        Icon::Shield => "◇",
        Icon::Clock => "◷",
        Icon::Memory => "▣",
        Icon::Monitor => "▭",
        Icon::Wrench => "⚒",
        Icon::Filter => "▽",
        Icon::More => "⋯",
        Icon::Pin => "⌖",
        Icon::Heart => "♥",
        Icon::Server => "▤",
        Icon::Palette => "◑",
        Icon::Keyboard => "⌨",
        Icon::Lock => "▣",
        Icon::Unlock => "▢",
        Icon::Upload => "↑",
        Icon::Camera => "◉",
        Icon::Sword => "⚔",
        Icon::Box => "▧",
        Icon::Link => "⛓",
        Icon::Power => "⏻",
    }
}

pub fn paint_scanline(ui: &Ui, rect: Rect, color: Color32) {
    let mut y = rect.top();
    while y < rect.bottom() {
        ui.painter().line_segment(
            [egui::pos2(rect.left(), y), egui::pos2(rect.right(), y)],
            Stroke::new(1.0, Theme::alpha(color, 12)),
        );
        y += 4.0;
    }
}

pub fn hairline(ui: &mut Ui, theme: &Theme) {
    let (rect, _) = ui.allocate_exact_size(Vec2::new(ui.available_width(), 1.0), Sense::hover());
    ui.painter()
        .rect_filled(rect, 0.0, Theme::alpha(theme.outline, 120));
}

pub fn panel_shadow(ui: &Ui, rect: Rect, theme: &Theme) {
    ui.painter().rect_stroke(
        rect,
        16.0,
        Stroke::new(1.0, Theme::alpha(theme.primary, 34)),
        StrokeKind::Inside,
    );
}
