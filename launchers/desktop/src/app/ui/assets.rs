use eframe::egui;
use egui::{ColorImage, Context, TextureHandle, TextureOptions};

pub fn background(ctx: &Context) -> Option<TextureHandle> {
    decode(
        ctx,
        "matticraft-background",
        include_bytes!("../../../assets/background.jpeg"),
        TextureOptions::LINEAR,
    )
}

pub fn steve(ctx: &Context) -> Option<TextureHandle> {
    decode(
        ctx,
        "matticraft-steve",
        include_bytes!("../../../assets/steve.png"),
        TextureOptions::NEAREST,
    )
}

fn decode(
    ctx: &Context,
    name: &str,
    bytes: &[u8],
    options: TextureOptions,
) -> Option<TextureHandle> {
    let image = image::load_from_memory(bytes).ok()?.to_rgba8();
    let size = [image.width() as usize, image.height() as usize];
    let color = ColorImage::from_rgba_unmultiplied(size, image.as_raw());
    Some(ctx.load_texture(name, color, options))
}

pub fn paint_background(
    ui: &egui::Ui,
    texture: Option<&TextureHandle>,
    dim: f32,
    tint: egui::Color32,
) {
    let rect = ui.max_rect();
    if let Some(texture) = texture {
        ui.painter().image(
            texture.id(),
            rect,
            egui::Rect::from_min_max(egui::Pos2::ZERO, egui::pos2(1.0, 1.0)),
            egui::Color32::WHITE,
        );
    } else {
        ui.painter().rect_filled(rect, 0.0, tint);
    }
    ui.painter().rect_filled(
        rect,
        0.0,
        egui::Color32::from_black_alpha((dim.clamp(0.0, 1.0) * 255.0) as u8),
    );
    ui.painter().rect_filled(
        rect,
        0.0,
        egui::Color32::from_rgba_unmultiplied(
            tint.r(),
            tint.g(),
            tint.b(),
            (dim * 42.0) as u8,
        ),
    );
}
