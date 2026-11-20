use eframe::egui;
use egui::{
    Align, Align2, Color32, FontId, Layout, Rect, RichText, Sense, Stroke, Vec2,
};

use super::icons::{self, Icon};
use super::state::{ConfirmAction, ToastKind};
use super::theme::Theme;
use super::{assets, data, pages, widgets};
use crate::app::{LauncherApp, Tab};

const NAV_WIDTH: f32 = 218.0;

pub fn show(app: &mut LauncherApp, ctx: &egui::Context) {
    app.ui.tick(ctx);
    app.ensure_textures(ctx);
    shortcuts(app, ctx);
    top_bar(app, ctx);
    navigation(app, ctx);
    status_bar(app, ctx);
    central(app, ctx);
    command_palette(app, ctx);
    notification_center(app, ctx);
    confirm_dialog(app, ctx);
    toasts(app, ctx);
}

fn shortcuts(app: &mut LauncherApp, ctx: &egui::Context) {
    let command = ctx.input(|input| input.modifiers.command);
    let shift = ctx.input(|input| input.modifiers.shift);
    if command && ctx.input(|input| input.key_pressed(egui::Key::K)) {
        app.ui.command_open = true;
    }
    if command && shift && ctx.input(|input| input.key_pressed(egui::Key::M)) {
        app.tab = Tab::Mods;
    }
    if command && ctx.input(|input| input.key_pressed(egui::Key::Comma)) {
        app.tab = Tab::Settings;
    }
    if ctx.input(|input| input.key_pressed(egui::Key::F1)) {
        app.ui.shortcuts_open = true;
    }
    if ctx.input(|input| input.key_pressed(egui::Key::Escape)) {
        app.ui.command_open = false;
        app.ui.notifications_open = false;
        app.ui.shortcuts_open = false;
        app.ui.confirm = None;
        app.ui.add_profile_open = false;
        app.ui.add_account_open = false;
        app.ui.server_dialog_open = false;
    }
}

fn top_bar(app: &mut LauncherApp, ctx: &egui::Context) {
    let theme = app.theme;
    egui::TopBottomPanel::top("material3-top")
        .exact_height(68.0)
        .frame(
            egui::Frame::NONE
                .fill(theme.bg)
                .inner_margin(egui::Margin::symmetric(22, 12)),
        )
        .show(ctx, |ui| {
            ui.horizontal(|ui| {
                ui.vertical(|ui| {
                    ui.label(
                        RichText::new("MATTICRAFT")
                            .font(Theme::display_font(16.0))
                            .color(theme.primary)
                            .strong(),
                    );
                    ui.label(
                        RichText::new(format!(
                            "{}  /  {}",
                            app.tab.label(),
                            app.tab.subtitle()
                        ))
                        .font(theme.small())
                        .color(theme.on_surface_variant),
                    );
                });
                ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                    let account = app
                        .store
                        .active_account()
                        .map(|account| account.username.clone())
                        .unwrap_or_else(|| "Guest".into());
                    let response = ui
                        .add(
                            egui::Button::new(
                                RichText::new(format!("  {}  ", account))
                                    .font(theme.small())
                                    .color(theme.on_surface),
                            )
                            .fill(theme.surface_variant)
                            .stroke(Stroke::new(1.0, theme.outline))
                            .corner_radius(egui::CornerRadius::same(18)),
                        )
                        .on_hover_text("Active account");
                    if response.clicked() {
                        app.tab = Tab::Accounts;
                    }
                    widgets::avatar(ui, &theme, &account, 30.0);
                    if icons::button(ui, Icon::Bell, 32.0, false, &theme).clicked() {
                        app.ui.notifications_open = !app.ui.notifications_open;
                    }
                    let search = ui.add(
                        egui::Button::new(
                            RichText::new("⌕  Search or jump…   Ctrl K")
                                .font(theme.small())
                                .color(theme.on_surface_variant),
                        )
                        .fill(theme.surface)
                        .stroke(Stroke::new(1.0, theme.outline))
                        .corner_radius(egui::CornerRadius::same(16)),
                    );
                    if search.clicked() {
                        app.ui.command_open = true;
                    }
                    if icons::button(ui, Icon::Keyboard, 32.0, false, &theme).clicked() {
                        app.ui.shortcuts_open = true;
                    }
                });
            });
        });
}

fn navigation(app: &mut LauncherApp, ctx: &egui::Context) {
    let theme = app.theme;
    egui::SidePanel::left("material3-navigation")
        .resizable(false)
        .exact_width(NAV_WIDTH)
        .frame(
            egui::Frame::NONE
                .fill(theme.surface)
                .stroke(Stroke::new(1.0, theme.outline))
                .inner_margin(egui::Margin::symmetric(13, 14)),
        )
        .show(ctx, |ui| {
            ui.horizontal(|ui| {
                let (rect, _) = ui.allocate_exact_size(Vec2::splat(34.0), Sense::hover());
                ui.painter()
                    .rect_filled(rect, 10.0, theme.primary_container);
                ui.painter().text(
                    rect.center(),
                    Align2::CENTER_CENTER,
                    "M",
                    FontId::proportional(19.0),
                    theme.primary,
                );
                ui.vertical(|ui| {
                    ui.label(
                        RichText::new("MATTICRAFT")
                            .font(Theme::display_font(13.0))
                            .color(theme.on_surface),
                    );
                    ui.label(
                        RichText::new("DESKTOP CLIENT")
                            .font(theme.micro())
                            .color(theme.primary),
                    );
                });
            });
            ui.add_space(20.0);
            nav_section(ui, app, &theme, "WORKSPACE", &[Tab::Home, Tab::Library, Tab::Profiles, Tab::Mods, Tab::Studio]);
            ui.add_space(12.0);
            nav_section(ui, app, &theme, "COMMUNITY", &[Tab::Servers, Tab::Accounts, Tab::Cosmetics, Tab::News]);
            ui.add_space(12.0);
            nav_section(ui, app, &theme, "SYSTEM", &[Tab::Downloads, Tab::Logs, Tab::Settings, Tab::About]);

            ui.with_layout(Layout::bottom_up(Align::LEFT), |ui| {
                ui.add_space(4.0);
                let account = app
                    .store
                    .active_account()
                    .map(|account| account.username.clone())
                    .unwrap_or_else(|| "Guest".into());
                let frame = egui::Frame::NONE
                    .fill(theme.surface_variant)
                    .stroke(Stroke::new(1.0, theme.outline))
                    .corner_radius(egui::CornerRadius::same(14))
                    .inner_margin(egui::Margin::same(9));
                frame.show(ui, |ui| {
                    ui.horizontal(|ui| {
                        widgets::avatar(ui, &theme, &account, 28.0);
                        ui.vertical(|ui| {
                            ui.label(
                                RichText::new(&account)
                                    .font(theme.small())
                                    .strong()
                                    .color(theme.on_surface),
                            );
                            ui.label(
                                RichText::new("ACTIVE ACCOUNT")
                                    .font(theme.micro())
                                    .color(theme.on_surface_variant),
                            );
                        });
                        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                            if icons::button(ui, Icon::Settings, 22.0, false, &theme).clicked() {
                                app.tab = Tab::Accounts;
                            }
                        });
                    });
                });
                ui.add_space(8.0);
                ui.label(
                    RichText::new(format!(
                        "{} {}  ·  v{}",
                        crate::platform::Platform::current().name(),
                        crate::platform::arch(),
                        env!("CARGO_PKG_VERSION")
                    ))
                    .font(theme.micro())
                    .color(theme.faint),
                );
            });
        });
}

fn nav_section(ui: &mut egui::Ui, app: &mut LauncherApp, theme: &Theme, title: &str, tabs: &[Tab]) {
    ui.label(
        RichText::new(title)
            .font(theme.micro())
            .color(theme.faint)
            .strong(),
    );
    ui.add_space(4.0);
    for tab in tabs {
        let active = app.tab == *tab;
        let response = nav_button(ui, theme, *tab, active);
        if response.clicked() {
            app.tab = *tab;
            app.ui.page_enter = 0.0;
            app.ui.last_tab_switch = app.ui.now;
        }
    }
}

fn nav_button(ui: &mut egui::Ui, theme: &Theme, tab: Tab, active: bool) -> egui::Response {
    let (rect, response) = ui.allocate_exact_size(
        Vec2::new(ui.available_width(), 38.0),
        Sense::click(),
    );
    let fill = if active {
        theme.primary_container
    } else if response.hovered() {
        theme.card_hover
    } else {
        Color32::TRANSPARENT
    };
    ui.painter().rect_filled(rect, 11.0, fill);
    if active {
        ui.painter().rect_stroke(
            rect,
            11.0,
            Stroke::new(1.0, theme.primary),
            egui::StrokeKind::Inside,
        );
    }
    let icon_rect = Rect::from_min_size(
        egui::pos2(rect.left() + 12.0, rect.center().y - 8.0),
        Vec2::splat(16.0),
    );
    icons::paint(
        ui.painter(),
        icon_rect,
        icon_for(tab),
        if active {
            theme.primary
        } else {
            theme.on_surface_variant
        },
    );
    ui.painter().text(
        egui::pos2(rect.left() + 38.0, rect.center().y),
        Align2::LEFT_CENTER,
        tab.label(),
        FontId::proportional(11.0),
        if active {
            theme.primary
        } else {
            theme.on_surface
        },
    );
    if active {
        ui.painter().circle_filled(
            egui::pos2(rect.right() - 12.0, rect.center().y),
            2.6,
            theme.primary,
        );
    }
    response
}

fn icon_for(tab: Tab) -> Icon {
    match tab {
        Tab::Home => Icon::Home,
        Tab::Library => Icon::Grid,
        Tab::Profiles => Icon::Play,
        Tab::Mods => Icon::Layers,
        Tab::Studio => Icon::Wrench,
        Tab::Accounts => Icon::Users,
        Tab::Servers => Icon::Globe,
        Tab::Cosmetics => Icon::Sparkles,
        Tab::News => Icon::News,
        Tab::Downloads => Icon::Download,
        Tab::Logs => Icon::Terminal,
        Tab::Settings => Icon::Settings,
        Tab::About => Icon::Info,
    }
}

fn central(app: &mut LauncherApp, ctx: &egui::Context) {
    let theme = app.theme;
    egui::CentralPanel::default()
        .frame(
            egui::Frame::NONE
                .fill(theme.bg)
                .inner_margin(egui::Margin::symmetric(24, 18)),
        )
        .show(ctx, |ui| {
            if app.tab == Tab::Home {
                assets::paint_background(
                    ui,
                    app.background.as_ref(),
                    app.ui.background_dim,
                    theme.bg,
                );
            }
            egui::ScrollArea::vertical()
                .id_salt(format!("page-{:?}", app.tab))
                .auto_shrink([false, false])
                .show(ui, |ui| {
                    let alpha = if app.ui.animations {
                        app.ui.page_enter.clamp(0.0, 1.0)
                    } else {
                        1.0
                    };
                    ui.set_opacity(alpha);
                    pages::show(app, ui);
                });
        });
}

fn status_bar(app: &mut LauncherApp, ctx: &egui::Context) {
    let theme = app.theme;
    egui::TopBottomPanel::bottom("material3-status")
        .exact_height(34.0)
        .frame(
            egui::Frame::NONE
                .fill(theme.surface)
                .stroke(Stroke::new(1.0, theme.outline))
                .inner_margin(egui::Margin::symmetric(18, 6)),
        )
        .show(ctx, |ui| {
            ui.horizontal(|ui| {
                let color = if app.status.contains("failed") || app.status.contains("Cannot") {
                    theme.error
                } else {
                    theme.on_surface_variant
                };
                ui.label(
                    RichText::new(if app.status.is_empty() {
                        "Ready"
                    } else {
                        app.status.as_str()
                    })
                    .font(theme.micro())
                    .color(color),
                );
                ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                    if let Some(pid) = app.child_pid {
                        widgets::badge(ui, &theme, &format!("PID {pid}"), theme.primary);
                    }
                    ui.label(
                        RichText::new(format!("channel {}", app.store.settings.channel))
                            .font(theme.micro())
                            .color(theme.faint),
                    );
                    ui.label(
                        RichText::new(format!(
                            "{} builds",
                            app.game_versions.len()
                        ))
                        .font(theme.micro())
                        .color(theme.faint),
                    );
                });
            });
        });
}

fn command_palette(app: &mut LauncherApp, ctx: &egui::Context) {
    if !app.ui.command_open {
        return;
    }
    let theme = app.theme;
    let mut open = app.ui.command_open;
    egui::Window::new("command-palette")
        .id(egui::Id::new("command-palette"))
        .title_bar(false)
        .resizable(false)
        .collapsible(false)
        .anchor(Align2::CENTER_TOP, [0.0, 84.0])
        .fixed_size([620.0, 430.0])
        .frame(super::theme::modal_frame(&theme))
        .open(&mut open)
        .show(ctx, |ui| {
            ui.horizontal(|ui| {
                icons::paint(
                    ui.painter(),
                    Rect::from_min_size(ui.cursor().min, Vec2::splat(18.0)),
                    Icon::Search,
                    theme.primary,
                );
                ui.add_space(24.0);
                let response = ui.add(
                    egui::TextEdit::singleline(&mut app.ui.command_query)
                        .hint_text("Search pages, profiles and actions…")
                        .frame(false)
                        .desired_width(f32::INFINITY),
                );
                response.request_focus();
            });
            widgets::divider(ui, &theme);
            let items = command_items(app);
            let filtered: Vec<_> = items
                .into_iter()
                .filter(|item| app.ui.command_matches(&app.ui.command_query, item.label, item.hint))
                .collect();
            egui::ScrollArea::vertical()
                .max_height(330.0)
                .show(ui, |ui| {
                    for item in filtered {
                        let response = ui.add_sized(
                            [ui.available_width(), 34.0],
                            egui::Button::new(
                                RichText::new(format!("{}   {}", item.glyph, item.label))
                                    .font(theme.body())
                                    .color(theme.on_surface),
                            )
                            .fill(Color32::TRANSPARENT)
                            .stroke(Stroke::NONE),
                        );
                        if response.clicked() {
                            (item.action)(app);
                            app.ui.close_command();
                        }
                    }
                });
            ui.add_space(4.0);
            ui.label(
                RichText::new("Enter selects · Esc closes")
                    .font(theme.micro())
                    .color(theme.faint),
            );
        });
    app.ui.command_open = open;
}

struct CommandItem {
    label: &'static str,
    hint: &'static str,
    glyph: &'static str,
    action: fn(&mut LauncherApp),
}

fn command_items(app: &LauncherApp) -> Vec<CommandItem> {
    let mut items = Vec::new();
    for tab in Tab::ALL {
        items.push(CommandItem {
            label: tab.title(),
            hint: tab.subtitle(),
            glyph: tab.glyph(),
            action: match tab {
                Tab::Home => |app| app.tab = Tab::Home,
                Tab::Library => |app| app.tab = Tab::Library,
                Tab::Profiles => |app| app.tab = Tab::Profiles,
                Tab::Mods => |app| app.tab = Tab::Mods,
                Tab::Studio => |app| app.tab = Tab::Studio,
                Tab::Accounts => |app| app.tab = Tab::Accounts,
                Tab::Servers => |app| app.tab = Tab::Servers,
                Tab::Cosmetics => |app| app.tab = Tab::Cosmetics,
                Tab::News => |app| app.tab = Tab::News,
                Tab::Downloads => |app| app.tab = Tab::Downloads,
                Tab::Logs => |app| app.tab = Tab::Logs,
                Tab::Settings => |app| app.tab = Tab::Settings,
                Tab::About => |app| app.tab = Tab::About,
            },
        });
    }
    items.push(CommandItem {
        label: "New profile",
        hint: "Create a separate instance",
        glyph: "+",
        action: |app| app.ui.add_profile_open = true,
    });
    items.push(CommandItem {
        label: "Play selected profile",
        hint: "Start the game now",
        glyph: "▶",
        action: |app| app.play(),
    });
    items.push(CommandItem {
        label: "Refresh versions",
        hint: "Load the GitHub release list",
        glyph: "↻",
        action: |app| app.refresh_game_versions(),
    });
    items.push(CommandItem {
        label: "Check updates",
        hint: "Verify the signed release manifest",
        glyph: "◆",
        action: |app| app.check_updates(),
    });
    items.push(CommandItem {
        label: "Open data folder",
        hint: "Show the launcher data directory",
        glyph: "▱",
        action: |app| app.open_path(app.store.root().to_path_buf()),
    });
    items
}

fn notification_center(app: &mut LauncherApp, ctx: &egui::Context) {
    if !app.ui.notifications_open {
        return;
    }
    let theme = app.theme;
    egui::Window::new("notifications")
        .title_bar(false)
        .resizable(false)
        .collapsible(false)
        .anchor(Align2::RIGHT_TOP, [-26.0, 74.0])
        .fixed_size([360.0, 300.0])
        .frame(super::theme::modal_frame(&theme))
        .show(ctx, |ui| {
            ui.horizontal(|ui| {
                ui.label(
                    RichText::new("NOTIFICATIONS")
                        .font(Theme::display_font(12.0))
                        .color(theme.primary),
                );
                ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                    if ui.small_button("×").clicked() {
                        app.ui.notifications_open = false;
                    }
                });
            });
            ui.add_space(8.0);
            if app.ui.toasts.is_empty() {
                widgets::empty_state(
                    ui,
                    &theme,
                    Icon::Bell,
                    "Nothing new",
                    "Launcher events and completed actions appear here.",
                );
            } else {
                for toast in app.ui.toasts.iter().rev() {
                    ui.horizontal(|ui| {
                        widgets::badge(
                            ui,
                            &theme,
                            toast.kind.label(),
                            toast_color(toast.kind, &theme),
                        );
                        ui.label(
                            RichText::new(&toast.title)
                                .font(theme.small())
                                .strong()
                                .color(theme.on_surface),
                        );
                    });
                    ui.label(
                        RichText::new(&toast.message)
                            .font(theme.small())
                            .color(theme.on_surface_variant),
                    );
                    ui.add_space(6.0);
                }
            }
        });
}

fn toast_color(kind: ToastKind, theme: &Theme) -> Color32 {
    match kind {
        ToastKind::Info => theme.info,
        ToastKind::Success => theme.success,
        ToastKind::Warning => theme.warning,
        ToastKind::Error => theme.error,
    }
}

fn confirm_dialog(app: &mut LauncherApp, ctx: &egui::Context) {
    let Some(dialog) = app.ui.confirm.clone() else {
        return;
    };
    let theme = app.theme;
    let mut open = true;
    egui::Window::new("confirm")
        .title_bar(false)
        .resizable(false)
        .collapsible(false)
        .anchor(Align2::CENTER_CENTER, [0.0, 0.0])
        .fixed_size([440.0, 190.0])
        .frame(super::theme::modal_frame(&theme))
        .open(&mut open)
        .show(ctx, |ui| {
            ui.label(
                RichText::new(&dialog.title)
                    .font(theme.subheading())
                    .strong()
                    .color(theme.on_surface),
            );
            ui.add_space(5.0);
            ui.label(
                RichText::new(&dialog.message)
                    .font(theme.body())
                    .color(theme.on_surface_variant),
            );
            ui.add_space(18.0);
            ui.horizontal(|ui| {
                if dialog.destructive {
                    if widgets::danger_button(ui, &theme, &dialog.confirm_label).clicked() {
                        apply_confirm(app, &dialog.action);
                        app.ui.confirm = None;
                    }
                } else if widgets::primary_button(ui, &theme, &dialog.confirm_label, None).clicked() {
                    apply_confirm(app, &dialog.action);
                    app.ui.confirm = None;
                }
                if widgets::secondary_button(ui, &theme, "Cancel", None).clicked() {
                    app.ui.confirm = None;
                }
            });
        });
    if !open {
        app.ui.confirm = None;
    }
}

fn apply_confirm(app: &mut LauncherApp, action: &ConfirmAction) {
    match action {
        ConfirmAction::RemoveProfile(id) => {
            app.selected = Some(id.clone());
            app.remove_profile();
        }
        ConfirmAction::RemoveAccount(id) => {
            let _ = app.store.remove_account(id);
            let _ = app.store.save();
        }
        ConfirmAction::ResetPreferences => {
            app.ui.reset_preferences();
            app.theme = Theme::matticraft();
        }
        ConfirmAction::InstallGame(version) => {
            if let Some(release) = app
                .game_versions
                .iter()
                .find(|release| release.version == *version)
                .cloned()
            {
                app.install_game(&release);
            }
        }
        ConfirmAction::ApplyPatch(_) => app.apply_update_and_play(),
    }
}

fn toasts(app: &mut LauncherApp, ctx: &egui::Context) {
    if app.ui.toasts.is_empty() {
        return;
    }
    let theme = app.theme;
    egui::Area::new(egui::Id::new("toasts"))
        .order(egui::Order::Foreground)
        .anchor(Align2::RIGHT_BOTTOM, [-24.0, -52.0])
        .show(ctx, |ui| {
            for toast in app.ui.toasts.iter().rev().take(3) {
                let color = toast_color(toast.kind, &theme);
                egui::Frame::NONE
                    .fill(theme.surface)
                    .stroke(Stroke::new(1.0, color))
                    .corner_radius(egui::CornerRadius::same(14))
                    .inner_margin(egui::Margin::same(12))
                    .show(ui, |ui| {
                        ui.set_width(300.0);
                        ui.horizontal(|ui| {
                            widgets::badge(ui, &theme, toast.kind.label(), color);
                            ui.label(
                                RichText::new(&toast.title)
                                    .font(theme.small())
                                    .strong()
                                    .color(theme.on_surface),
                            );
                        });
                        ui.label(
                            RichText::new(&toast.message)
                                .font(theme.small())
                                .color(theme.on_surface_variant),
                        );
                    });
                ui.add_space(6.0);
            }
        });
}

pub fn section_hint(app: &LauncherApp, ui: &mut egui::Ui, text: &str) {
    ui.label(
        RichText::new(text)
            .font(app.theme.micro())
            .color(app.theme.faint),
    );
}

pub fn data_version() -> &'static str {
    data::NEWS[0].date
}
