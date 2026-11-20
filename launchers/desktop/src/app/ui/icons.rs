use eframe::egui;
use egui::{Color32, Pos2, Rect, Response, Sense, Stroke, StrokeKind, Vec2};

use super::theme::Theme;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Icon {
    Home,
    Play,
    Grid,
    Layers,
    User,
    Users,
    Globe,
    Sparkles,
    News,
    Download,
    Terminal,
    Settings,
    Info,
    Search,
    Bell,
    Plus,
    Minus,
    Close,
    Check,
    Warning,
    Refresh,
    Folder,
    Trash,
    Copy,
    External,
    ChevronRight,
    ChevronDown,
    ArrowLeft,
    ArrowRight,
    Shield,
    Clock,
    Memory,
    Monitor,
    Wrench,
    Filter,
    More,
    Pin,
    Heart,
    Server,
    Palette,
    Keyboard,
    Lock,
    Unlock,
    Upload,
    Camera,
    Sword,
    Box,
    Link,
    Power,
}

impl Icon {
    pub fn label(self) -> &'static str {
        match self {
            Icon::Home => "Home",
            Icon::Play => "Play",
            Icon::Grid => "Grid",
            Icon::Layers => "Layers",
            Icon::User => "Account",
            Icon::Users => "Accounts",
            Icon::Globe => "Servers",
            Icon::Sparkles => "Cosmetics",
            Icon::News => "News",
            Icon::Download => "Downloads",
            Icon::Terminal => "Logs",
            Icon::Settings => "Settings",
            Icon::Info => "About",
            Icon::Search => "Search",
            Icon::Bell => "Notifications",
            Icon::Plus => "Add",
            Icon::Minus => "Remove",
            Icon::Close => "Close",
            Icon::Check => "Done",
            Icon::Warning => "Warning",
            Icon::Refresh => "Refresh",
            Icon::Folder => "Folder",
            Icon::Trash => "Delete",
            Icon::Copy => "Copy",
            Icon::External => "Open",
            Icon::ChevronRight => "Next",
            Icon::ChevronDown => "Expand",
            Icon::ArrowLeft => "Back",
            Icon::ArrowRight => "Forward",
            Icon::Shield => "Security",
            Icon::Clock => "History",
            Icon::Memory => "Memory",
            Icon::Monitor => "Display",
            Icon::Wrench => "Repair",
            Icon::Filter => "Filter",
            Icon::More => "More",
            Icon::Pin => "Pin",
            Icon::Heart => "Favorite",
            Icon::Server => "Server",
            Icon::Palette => "Theme",
            Icon::Keyboard => "Keys",
            Icon::Lock => "Locked",
            Icon::Unlock => "Unlocked",
            Icon::Upload => "Upload",
            Icon::Camera => "Screenshot",
            Icon::Sword => "Combat",
            Icon::Box => "Resource pack",
            Icon::Link => "Link",
            Icon::Power => "Quit",
        }
    }
}

pub fn button(
    ui: &mut egui::Ui,
    icon: Icon,
    size: f32,
    active: bool,
    theme: &Theme,
) -> Response {
    let (rect, response) = ui.allocate_exact_size(Vec2::splat(size), Sense::click());
    let fill = if active {
        theme.primary_container
    } else if response.hovered() {
        theme.surface_variant
    } else {
        Color32::TRANSPARENT
    };
    let stroke = if active || response.hovered() {
        Stroke::new(1.0, if active { theme.primary } else { theme.outline })
    } else {
        Stroke::NONE
    };
    ui.painter().rect_filled(rect, 10.0, fill);
    if stroke != Stroke::NONE {
        ui.painter()
            .rect_stroke(rect, 10.0, stroke, StrokeKind::Inside);
    }
    let color = if active {
        theme.primary
    } else if response.hovered() {
        theme.on_surface
    } else {
        theme.on_surface_variant
    };
    paint(ui.painter(), rect.shrink(size * 0.28), icon, color);
    response.on_hover_text(icon.label())
}

pub fn paint(painter: &egui::Painter, rect: Rect, icon: Icon, color: Color32) {
    let stroke = Stroke::new(1.7_f32.max(rect.width() * 0.08), color);
    let center = rect.center();
    let left = rect.left();
    let right = rect.right();
    let top = rect.top();
    let bottom = rect.bottom();
    let mid_y = center.y;
    let quarter = rect.width() * 0.25;
    let line = |a: Pos2, b: Pos2| painter.line_segment([a, b], stroke);
    let dot = |x: f32, y: f32, radius: f32| painter.circle_filled(Pos2::new(x, y), radius, color);
    match icon {
        Icon::Home => {
            line(Pos2::new(left, mid_y), Pos2::new(center.x, top));
            line(Pos2::new(center.x, top), Pos2::new(right, mid_y));
            line(Pos2::new(left + quarter, mid_y), Pos2::new(left + quarter, bottom));
            line(Pos2::new(right - quarter, mid_y), Pos2::new(right - quarter, bottom));
            line(Pos2::new(left + quarter, bottom), Pos2::new(right - quarter, bottom));
        }
        Icon::Play => {
            painter.add(egui::Shape::convex_polygon(
                vec![
                    Pos2::new(left + quarter, top),
                    Pos2::new(right - quarter * 0.5, mid_y),
                    Pos2::new(left + quarter, bottom),
                ],
                color,
                Stroke::NONE,
            ));
        }
        Icon::Grid => {
            let gap = rect.width() * 0.12;
            let cell = (rect.width() - gap) * 0.5;
            for x in [left, left + cell + gap] {
                for y in [top, top + cell + gap] {
                    painter.rect_filled(
                        Rect::from_min_size(Pos2::new(x, y), Vec2::splat(cell)),
                        3.0,
                        color,
                    );
                }
            }
        }
        Icon::Layers => {
            for offset in [-quarter * 0.7, 0.0, quarter * 0.7] {
                let y = mid_y + offset;
                line(Pos2::new(left, y), Pos2::new(center.x, y - quarter * 0.55));
                line(Pos2::new(center.x, y - quarter * 0.55), Pos2::new(right, y));
            }
        }
        Icon::User => {
            painter.circle_stroke(Pos2::new(center.x, top + quarter * 1.2), quarter * 0.75, stroke);
            painter.add(egui::Shape::line(
                vec![
                    Pos2::new(left + quarter * 0.65, bottom),
                    Pos2::new(left + quarter, mid_y + quarter * 0.45),
                    Pos2::new(right - quarter, mid_y + quarter * 0.45),
                    Pos2::new(right - quarter * 0.65, bottom),
                ],
                stroke,
            ));
        }
        Icon::Users => {
            painter.circle_stroke(Pos2::new(center.x - quarter * 0.6, top + quarter), quarter * 0.6, stroke);
            painter.circle_stroke(Pos2::new(center.x + quarter * 0.8, top + quarter * 1.2), quarter * 0.5, stroke);
            line(Pos2::new(left, bottom), Pos2::new(left + quarter * 0.9, mid_y + quarter * 0.8));
            line(Pos2::new(left + quarter * 0.9, mid_y + quarter * 0.8), Pos2::new(center.x + quarter, bottom));
            line(Pos2::new(center.x + quarter, bottom), Pos2::new(right, mid_y + quarter * 0.8));
        }
        Icon::Globe => {
            painter.circle_stroke(center, quarter * 1.8, stroke);
            painter.add(egui::Shape::ellipse_stroke(
                center,
                Vec2::new(quarter * 1.1, quarter * 1.8),
                stroke,
            ));
            line(Pos2::new(left + 1.0, mid_y), Pos2::new(right - 1.0, mid_y));
            line(Pos2::new(left + quarter * 0.45, top + quarter), Pos2::new(right - quarter * 0.45, top + quarter));
            line(Pos2::new(left + quarter * 0.45, bottom - quarter), Pos2::new(right - quarter * 0.45, bottom - quarter));
        }
        Icon::Sparkles => {
            let points = [
                Pos2::new(center.x, top),
                Pos2::new(center.x + quarter * 0.35, center.y - quarter * 0.35),
                Pos2::new(right, center.y),
                Pos2::new(center.x + quarter * 0.35, center.y + quarter * 0.35),
                Pos2::new(center.x, bottom),
                Pos2::new(center.x - quarter * 0.35, center.y + quarter * 0.35),
                Pos2::new(left, center.y),
                Pos2::new(center.x - quarter * 0.35, center.y - quarter * 0.35),
            ];
            painter.add(egui::Shape::convex_polygon(points.to_vec(), color, Stroke::NONE));
            dot(left + quarter * 0.5, top + quarter * 0.7, 1.2);
            dot(right - quarter * 0.4, bottom - quarter * 0.8, 1.2);
        }
        Icon::News => {
            painter.rect_stroke(rect.shrink(1.0), 3.0, stroke, StrokeKind::Inside);
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, top + quarter));
            for index in 0..3 {
                let y = top + quarter * 1.6 + index as f32 * quarter * 0.65;
                line(Pos2::new(left + quarter, y), Pos2::new(right - quarter * 0.4, y));
            }
        }
        Icon::Download => {
            line(Pos2::new(center.x, top), Pos2::new(center.x, mid_y + quarter * 0.7));
            line(Pos2::new(center.x, mid_y + quarter * 0.7), Pos2::new(center.x - quarter * 0.75, mid_y));
            line(Pos2::new(center.x, mid_y + quarter * 0.7), Pos2::new(center.x + quarter * 0.75, mid_y));
            line(Pos2::new(left + quarter * 0.6, bottom - quarter * 0.3), Pos2::new(right - quarter * 0.6, bottom - quarter * 0.3));
        }
        Icon::Terminal => {
            painter.rect_stroke(rect.shrink(1.0), 3.0, stroke, StrokeKind::Inside);
            line(Pos2::new(left + quarter * 0.7, top + quarter), Pos2::new(center.x, mid_y));
            line(Pos2::new(center.x, mid_y), Pos2::new(left + quarter * 0.7, bottom - quarter));
            line(Pos2::new(center.x + quarter * 0.5, bottom - quarter), Pos2::new(right - quarter * 0.7, bottom - quarter));
        }
        Icon::Settings => {
            painter.circle_stroke(center, quarter * 0.7, stroke);
            for index in 0..8 {
                let angle = std::f32::consts::TAU * index as f32 / 8.0;
                let from = center + Vec2::angled(angle) * quarter * 1.25;
                let to = center + Vec2::angled(angle) * quarter * 1.8;
                line(from, to);
            }
        }
        Icon::Info => {
            painter.circle_stroke(center, quarter * 1.8, stroke);
            line(Pos2::new(center.x, mid_y - quarter * 0.55), Pos2::new(center.x, mid_y + quarter * 0.9));
            dot(center.x, top + quarter * 0.63, 1.4);
        }
        Icon::Search => {
            painter.circle_stroke(Pos2::new(center.x - quarter * 0.35, mid_y - quarter * 0.35), quarter * 1.1, stroke);
            line(Pos2::new(center.x + quarter * 0.35, mid_y + quarter * 0.35), Pos2::new(right, bottom));
        }
        Icon::Bell => {
            painter.add(egui::Shape::line(
                vec![
                    Pos2::new(left + quarter, bottom - quarter * 0.2),
                    Pos2::new(left + quarter * 1.4, mid_y),
                    Pos2::new(left + quarter * 1.4, top + quarter),
                    Pos2::new(center.x, top),
                    Pos2::new(right - quarter * 1.4, top + quarter),
                    Pos2::new(right - quarter * 1.4, mid_y),
                    Pos2::new(right - quarter, bottom - quarter * 0.2),
                ],
                stroke,
            ));
            line(Pos2::new(left + quarter, bottom - quarter * 0.2), Pos2::new(right - quarter, bottom - quarter * 0.2));
            dot(center.x, bottom - quarter * 0.05, 1.5);
        }
        Icon::Plus | Icon::Minus => {
            line(Pos2::new(left + quarter, mid_y), Pos2::new(right - quarter, mid_y));
            if icon == Icon::Plus {
                line(Pos2::new(center.x, top + quarter), Pos2::new(center.x, bottom - quarter));
            }
        }
        Icon::Close => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, bottom - quarter));
            line(Pos2::new(right - quarter, top + quarter), Pos2::new(left + quarter, bottom - quarter));
        }
        Icon::Check => {
            line(Pos2::new(left + quarter, mid_y), Pos2::new(center.x, bottom - quarter));
            line(Pos2::new(center.x, bottom - quarter), Pos2::new(right - quarter, top + quarter));
        }
        Icon::Warning => {
            line(Pos2::new(center.x, top + quarter), Pos2::new(left + quarter, bottom - quarter));
            line(Pos2::new(left + quarter, bottom - quarter), Pos2::new(right - quarter, bottom - quarter));
            line(Pos2::new(right - quarter, bottom - quarter), Pos2::new(center.x, top + quarter));
            line(Pos2::new(center.x, mid_y - quarter * 0.1), Pos2::new(center.x, mid_y + quarter * 0.65));
            dot(center.x, bottom - quarter * 0.65, 1.3);
        }
        Icon::Refresh => {
            painter.circle_stroke(center, quarter * 1.5, stroke);
            line(Pos2::new(right - quarter * 0.45, top + quarter * 0.45), Pos2::new(right - quarter * 0.3, top + quarter * 1.2));
            line(Pos2::new(right - quarter * 0.45, top + quarter * 0.45), Pos2::new(left + quarter * 1.2, top + quarter * 0.7));
        }
        Icon::Folder => {
            painter.add(egui::Shape::line(
                vec![
                    Pos2::new(left, top + quarter),
                    Pos2::new(center.x - quarter * 0.2, top + quarter),
                    Pos2::new(center.x, top + quarter * 0.3),
                    Pos2::new(right, top + quarter * 0.3),
                    Pos2::new(right, bottom - quarter * 0.3),
                    Pos2::new(left, bottom - quarter * 0.3),
                    Pos2::new(left, top + quarter),
                ],
                stroke,
            ));
        }
        Icon::Trash => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, top + quarter));
            line(Pos2::new(center.x - quarter * 0.4, top + quarter * 0.45), Pos2::new(center.x + quarter * 0.4, top + quarter * 0.45));
            line(Pos2::new(left + quarter * 1.2, top + quarter), Pos2::new(left + quarter * 1.35, bottom - quarter));
            line(Pos2::new(right - quarter * 1.2, top + quarter), Pos2::new(right - quarter * 1.35, bottom - quarter));
            line(Pos2::new(left + quarter * 1.35, bottom - quarter), Pos2::new(right - quarter * 1.35, bottom - quarter));
        }
        Icon::Copy => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter, top), Pos2::new(right - quarter * 0.3, bottom - quarter)), 2.0, stroke, StrokeKind::Inside);
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter * 0.3, top + quarter), Pos2::new(right, bottom)), 2.0, stroke, StrokeKind::Inside);
        }
        Icon::External => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, bottom - quarter)), 2.0, stroke, StrokeKind::Inside);
            line(Pos2::new(center.x, mid_y), Pos2::new(right, top));
            line(Pos2::new(right - quarter, top), Pos2::new(right, top));
            line(Pos2::new(right, top), Pos2::new(right, top + quarter));
        }
        Icon::ChevronRight => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, mid_y));
            line(Pos2::new(right - quarter, mid_y), Pos2::new(left + quarter, bottom - quarter));
        }
        Icon::ChevronDown => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(center.x, bottom - quarter));
            line(Pos2::new(center.x, bottom - quarter), Pos2::new(right - quarter, top + quarter));
        }
        Icon::ArrowLeft => {
            line(Pos2::new(right - quarter, mid_y), Pos2::new(left + quarter, mid_y));
            line(Pos2::new(left + quarter, mid_y), Pos2::new(center.x - quarter * 0.4, top + quarter));
            line(Pos2::new(left + quarter, mid_y), Pos2::new(center.x - quarter * 0.4, bottom - quarter));
        }
        Icon::ArrowRight => {
            line(Pos2::new(left + quarter, mid_y), Pos2::new(right - quarter, mid_y));
            line(Pos2::new(right - quarter, mid_y), Pos2::new(center.x + quarter * 0.4, top + quarter));
            line(Pos2::new(right - quarter, mid_y), Pos2::new(center.x + quarter * 0.4, bottom - quarter));
        }
        Icon::Shield => {
            painter.add(egui::Shape::line(
                vec![
                    Pos2::new(center.x, top),
                    Pos2::new(right - quarter, top + quarter * 0.8),
                    Pos2::new(right - quarter * 1.2, bottom - quarter * 0.9),
                    Pos2::new(center.x, bottom),
                    Pos2::new(left + quarter * 1.2, bottom - quarter * 0.9),
                    Pos2::new(left + quarter, top + quarter * 0.8),
                    Pos2::new(center.x, top),
                ],
                stroke,
            ));
        }
        Icon::Clock => {
            painter.circle_stroke(center, quarter * 1.8, stroke);
            line(center, Pos2::new(center.x, center.y - quarter * 1.1));
            line(center, Pos2::new(center.x + quarter * 0.8, center.y + quarter * 0.35));
        }
        Icon::Memory => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter * 0.6, top + quarter), Pos2::new(right - quarter * 0.6, bottom - quarter)), 2.0, stroke, StrokeKind::Inside);
            for index in 0..4 {
                let x = left + quarter * (0.9 + index as f32 * 0.75);
                line(Pos2::new(x, top + quarter * 0.5), Pos2::new(x, top + quarter));
                line(Pos2::new(x, bottom - quarter), Pos2::new(x, bottom - quarter * 0.5));
            }
        }
        Icon::Monitor => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, bottom - quarter * 1.2)), 2.0, stroke, StrokeKind::Inside);
            line(Pos2::new(center.x, bottom - quarter * 1.2), Pos2::new(center.x, bottom - quarter * 0.35));
            line(Pos2::new(center.x - quarter, bottom - quarter * 0.35), Pos2::new(center.x + quarter, bottom - quarter * 0.35));
        }
        Icon::Wrench => {
            line(Pos2::new(left + quarter, bottom - quarter), Pos2::new(right - quarter * 1.1, top + quarter * 0.9));
            painter.circle_stroke(Pos2::new(right - quarter * 0.85, top + quarter * 0.7), quarter * 0.55, stroke);
            painter.circle_stroke(Pos2::new(left + quarter * 0.7, bottom - quarter * 0.7), quarter * 0.45, stroke);
        }
        Icon::Filter => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(right - quarter, top + quarter));
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(center.x, mid_y + quarter * 0.3));
            line(Pos2::new(right - quarter, top + quarter), Pos2::new(center.x, mid_y + quarter * 0.3));
            line(Pos2::new(center.x, mid_y + quarter * 0.3), Pos2::new(center.x, bottom - quarter));
        }
        Icon::More => {
            dot(left + quarter, mid_y, 1.8);
            dot(center.x, mid_y, 1.8);
            dot(right - quarter, mid_y, 1.8);
        }
        Icon::Pin => {
            line(Pos2::new(center.x, bottom), Pos2::new(center.x, mid_y));
            painter.add(egui::Shape::convex_polygon(
                vec![
                    Pos2::new(left + quarter, top + quarter),
                    Pos2::new(right - quarter, top + quarter),
                    Pos2::new(right - quarter * 0.65, mid_y),
                    Pos2::new(left + quarter * 0.65, mid_y),
                ],
                color,
                Stroke::NONE,
            ));
        }
        Icon::Heart => {
            painter.circle_filled(Pos2::new(left + quarter * 1.15, mid_y - quarter * 0.4), quarter * 0.65, color);
            painter.circle_filled(Pos2::new(right - quarter * 1.15, mid_y - quarter * 0.4), quarter * 0.65, color);
            painter.add(egui::Shape::convex_polygon(
                vec![
                    Pos2::new(left + quarter * 0.55, mid_y - quarter * 0.15),
                    Pos2::new(center.x, bottom - quarter * 0.3),
                    Pos2::new(right - quarter * 0.55, mid_y - quarter * 0.15),
                ],
                color,
                Stroke::NONE,
            ));
        }
        Icon::Server => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter * 0.6, top + quarter), Pos2::new(right - quarter * 0.6, mid_y - 1.0)), 2.0, stroke, StrokeKind::Inside);
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter * 0.6, mid_y + 1.0), Pos2::new(right - quarter * 0.6, bottom - quarter)), 2.0, stroke, StrokeKind::Inside);
            dot(left + quarter * 1.4, top + quarter * 1.65, 1.2);
            dot(left + quarter * 1.4, mid_y + quarter * 0.75, 1.2);
        }
        Icon::Palette => {
            painter.circle_stroke(center, quarter * 1.8, stroke);
            dot(center.x - quarter * 0.7, center.y - quarter * 0.7, 1.2);
            dot(center.x + quarter * 0.7, center.y - quarter * 0.5, 1.2);
            dot(center.x - quarter * 0.1, center.y + quarter * 0.8, 1.2);
        }
        Icon::Keyboard => {
            painter.rect_stroke(rect.shrink(1.0), 3.0, stroke, StrokeKind::Inside);
            for row in 0..2 {
                for column in 0..3 {
                    dot(
                        left + quarter * (1.0 + column as f32 * 0.9),
                        top + quarter * (1.2 + row as f32 * 0.7),
                        1.0,
                    );
                }
            }
        }
        Icon::Lock | Icon::Unlock => {
            painter.rect_filled(
                Rect::from_min_max(Pos2::new(left + quarter, mid_y - quarter * 0.2), Pos2::new(right - quarter, bottom - quarter * 0.2)),
                2.0,
                color,
            );
            painter.circle_stroke(Pos2::new(center.x, mid_y - quarter * 0.2), quarter * 0.9, stroke);
            if icon == Icon::Lock {
                line(Pos2::new(center.x - quarter * 0.9, mid_y - quarter * 0.2), Pos2::new(center.x - quarter * 0.9, mid_y - quarter * 1.1));
                line(Pos2::new(center.x + quarter * 0.9, mid_y - quarter * 0.2), Pos2::new(center.x + quarter * 0.9, mid_y - quarter * 1.1));
            }
        }
        Icon::Upload => {
            line(Pos2::new(center.x, bottom), Pos2::new(center.x, mid_y - quarter * 0.7));
            line(Pos2::new(center.x, mid_y - quarter * 0.7), Pos2::new(center.x - quarter * 0.75, mid_y));
            line(Pos2::new(center.x, mid_y - quarter * 0.7), Pos2::new(center.x + quarter * 0.75, mid_y));
            line(Pos2::new(left + quarter * 0.6, bottom - quarter * 0.3), Pos2::new(right - quarter * 0.6, bottom - quarter * 0.3));
        }
        Icon::Camera => {
            painter.rect_stroke(Rect::from_min_max(Pos2::new(left + quarter * 0.55, top + quarter * 1.1), Pos2::new(right - quarter * 0.55, bottom - quarter * 0.45)), 3.0, stroke, StrokeKind::Inside);
            painter.circle_stroke(center, quarter * 0.65, stroke);
            line(Pos2::new(left + quarter * 1.1, top + quarter * 1.1), Pos2::new(left + quarter * 1.45, top + quarter * 0.55));
            line(Pos2::new(left + quarter * 1.45, top + quarter * 0.55), Pos2::new(left + quarter * 1.9, top + quarter * 0.55));
        }
        Icon::Sword => {
            line(Pos2::new(left + quarter, bottom - quarter), Pos2::new(right - quarter * 0.8, top + quarter * 0.8));
            line(Pos2::new(left + quarter * 0.5, mid_y), Pos2::new(center.x + quarter * 0.2, bottom - quarter * 0.4));
            line(Pos2::new(left + quarter * 0.8, bottom - quarter * 0.5), Pos2::new(right - quarter * 0.5, bottom - quarter));
        }
        Icon::Box => {
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(center.x, top));
            line(Pos2::new(center.x, top), Pos2::new(right - quarter, top + quarter));
            line(Pos2::new(right - quarter, top + quarter), Pos2::new(right - quarter, bottom - quarter));
            line(Pos2::new(right - quarter, bottom - quarter), Pos2::new(center.x, bottom));
            line(Pos2::new(center.x, bottom), Pos2::new(left + quarter, bottom - quarter));
            line(Pos2::new(left + quarter, bottom - quarter), Pos2::new(left + quarter, top + quarter));
            line(Pos2::new(left + quarter, top + quarter), Pos2::new(center.x, mid_y));
            line(Pos2::new(center.x, mid_y), Pos2::new(right - quarter, top + quarter));
        }
        Icon::Link => {
            painter.circle_stroke(Pos2::new(center.x - quarter * 0.55, mid_y + quarter * 0.5), quarter * 0.85, stroke);
            painter.circle_stroke(Pos2::new(center.x + quarter * 0.55, mid_y - quarter * 0.5), quarter * 0.85, stroke);
            line(Pos2::new(left + quarter * 0.9, bottom - quarter * 0.9), Pos2::new(right - quarter * 0.9, top + quarter * 0.9));
        }
        Icon::Power => {
            painter.circle_stroke(center, quarter * 1.55, stroke);
            line(Pos2::new(center.x, top), Pos2::new(center.x, mid_y));
        }
    }
}
