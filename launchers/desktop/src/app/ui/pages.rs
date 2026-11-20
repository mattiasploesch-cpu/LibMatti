use eframe::egui;
use egui::{Align, Color32, FontId, Layout, RichText, Stroke, Vec2};

use super::data::{self, COSMETICS, FAQ, NEWS, SERVERS, SHORTCUTS, TIPS};
use super::icons::{self, Icon};
use super::state::{
    ConfirmAction, CosmeticCategory, LibraryFilter, LogLevel, NewsCategory, ProfileLayout,
    SettingsSection,
};
use super::theme::{Theme, ThemePreset};
use super::widgets;
use crate::app::{LauncherApp, Tab};

pub fn show(app: &mut LauncherApp, ui: &mut egui::Ui) {
    match app.tab {
        Tab::Home => home(app, ui),
        Tab::Library => library(app, ui),
        Tab::Profiles => profiles(app, ui),
        Tab::Mods => mods(app, ui),
        Tab::Studio => studio_page(app, ui),
        Tab::Accounts => accounts(app, ui),
        Tab::Servers => servers(app, ui),
        Tab::Cosmetics => cosmetics(app, ui),
        Tab::News => news(app, ui),
        Tab::Downloads => downloads(app, ui),
        Tab::Logs => logs(app, ui),
        Tab::Settings => settings(app, ui),
        Tab::About => about(app, ui),
    }
}

fn home(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    let profile = app.draft.clone();
    let account = app
        .store
        .active_account()
        .map(|account| account.username.clone())
        .unwrap_or_else(|| "Guest".into());
    widgets::page_header(
        ui,
        &theme,
        "MATTICRAFT / HOME",
        "Your world starts here",
        "One profile, one click, the next world",
        |ui| {
            if widgets::primary_button(ui, &theme, "Play now", Some(Icon::Play)).clicked() {
                app.play();
            }
        },
    );
    ui.add_space(14.0);

    if let Some(profile) = profile {
        egui::Frame::NONE
            .fill(Color32::from_rgba_unmultiplied(20, 27, 21, 238))
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(22))
            .inner_margin(egui::Margin::same(24))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    ui.vertical(|ui| {
                        widgets::label(ui, &theme, "READY TO PLAY", theme.primary, 9.5, true);
                        ui.label(
                            RichText::new(&profile.name)
                                .font(FontId::proportional(34.0))
                                .strong()
                                .color(theme.on_surface),
                        );
                        ui.label(
                            RichText::new(format!(
                                "{}  ·  {}  ·  {} × {}",
                                profile.game_version,
                                profile.display_selection(),
                                profile.width,
                                profile.height
                            ))
                            .font(theme.body())
                            .color(theme.on_surface_variant),
                        );
                        ui.add_space(16.0);
                        ui.horizontal(|ui| {
                            if widgets::primary_button(ui, &theme, "Launch", Some(Icon::Play)).clicked() {
                                app.play();
                            }
                            if widgets::secondary_button(ui, &theme, "Edit profile", Some(Icon::Settings)).clicked() {
                                app.tab = Tab::Profiles;
                            }
                        });
                    });
                    ui.with_layout(Layout::right_to_left(Align::Min), |ui| {
                        egui::Frame::NONE
                            .fill(theme.primary_container)
                            .stroke(Stroke::new(1.0, theme.primary))
                            .corner_radius(egui::CornerRadius::same(18))
                            .inner_margin(egui::Margin::same(15))
                            .show(ui, |ui| {
                                ui.set_width(210.0);
                                widgets::label(ui, &theme, "ACTIVE ACCOUNT", theme.on_primary_container, 9.0, true);
                                ui.label(
                                    RichText::new(&account)
                                        .font(theme.subheading())
                                        .strong()
                                        .color(theme.on_primary_container),
                                );
                                ui.add_space(10.0);
                                widgets::key_value(ui, &theme, "Memory", &format!("{} MiB", profile.memory_mb));
                                widgets::key_value(ui, &theme, "Mods", &format!("{} file(s)", profile.mods_dir.read_dir().map(|entries| entries.count()).unwrap_or(0)));
                                ui.add_space(8.0);
                                if widgets::text_button(ui, &theme, "Manage account", Some(Icon::Users)).clicked() {
                                    app.tab = Tab::Accounts;
                                }
                            });
                    });
                });
            });
    }

    ui.add_space(16.0);
    ui.columns(3, |columns| {
        widgets::stat(&mut columns[0], &theme, &format!("{}", app.store.profiles.len()), "profiles", Icon::Layers);
        widgets::stat(&mut columns[1], &theme, &format!("{}", app.game_versions.len()), "builds available", Icon::Download);
        widgets::stat(
            &mut columns[2],
            &theme,
            &format!(
                "{}",
                app.mod_validation
                    .as_ref()
                    .map(|validation| validation.installed.len())
                    .unwrap_or(0)
            ),
            "mods installed",
            Icon::Box,
        );
    });

    ui.add_space(20.0);
    ui.columns(2, |columns| {
        columns[0].horizontal(|ui| {
            ui.label(
                RichText::new("QUICK ACTIONS")
                    .font(Theme::display_font(11.0))
                    .color(theme.primary),
            );
        });
        columns[0].add_space(7.0);
        for (title, hint, icon, tab) in [
            ("Game library", "Install another build", Icon::Grid, Tab::Library),
            ("Manage mods", "Validate the instance folder", Icon::Layers, Tab::Mods),
            ("Servers", "Join a saved world", Icon::Globe, Tab::Servers),
            ("Cosmetics", "Change your look", Icon::Sparkles, Tab::Cosmetics),
        ] {
            let response = egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(13))
                .inner_margin(egui::Margin::symmetric(12, 9))
                .show(&mut columns[0], |ui| {
                    ui.horizontal(|ui| {
                        icons::paint(
                            ui.painter(),
                            egui::Rect::from_min_size(ui.cursor().min, Vec2::splat(16.0)),
                            icon,
                            theme.primary,
                        );
                        ui.add_space(24.0);
                        ui.vertical(|ui| {
                            ui.label(RichText::new(title).font(theme.small()).strong().color(theme.on_surface));
                            ui.label(RichText::new(hint).font(theme.micro()).color(theme.on_surface_variant));
                        });
                    });
                })
                .response
                .interact(egui::Sense::click());
            if response.clicked() {
                app.tab = tab;
            }
            columns[0].add_space(5.0);
        }

        columns[1].label(
            RichText::new("LATEST NEWS")
                .font(Theme::display_font(11.0))
                .color(theme.primary),
        );
        columns[1].add_space(7.0);
        for item in NEWS.iter().take(3) {
            egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(13))
                .inner_margin(egui::Margin::same(11))
                .show(&mut columns[1], |ui| {
                    ui.horizontal(|ui| {
                        widgets::badge(ui, &theme, data::category_label(item.category), theme.secondary);
                        ui.label(RichText::new(item.date).font(theme.micro()).color(theme.faint));
                    });
                    ui.label(RichText::new(item.title).font(theme.body()).strong().color(theme.on_surface));
                    ui.label(RichText::new(item.summary).font(theme.small()).color(theme.on_surface_variant));
                });
            columns[1].add_space(6.0);
        }
        if widgets::text_button(&mut columns[1], &theme, "Read all news", Some(Icon::ArrowRight)).clicked() {
            app.tab = Tab::News;
        }
    });

    ui.add_space(18.0);
    egui::Frame::NONE
        .fill(Theme::alpha(theme.primary, 16))
        .stroke(Stroke::new(1.0, Theme::alpha(theme.primary, 90)))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(14))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                icons::paint(
                    ui.painter(),
                    egui::Rect::from_min_size(ui.cursor().min, Vec2::splat(18.0)),
                    Icon::Sparkles,
                    theme.primary,
                );
                ui.add_space(26.0);
                ui.vertical(|ui| {
                    ui.label(RichText::new(TIPS[0].title).font(theme.small()).strong().color(theme.on_surface));
                    ui.label(RichText::new(TIPS[0].body).font(theme.small()).color(theme.on_surface_variant));
                });
                ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                    if ui.small_button("Next tip").clicked() {
                        app.ui.info("Tip", TIPS[1].body);
                    }
                });
            });
        });
}

fn library(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "BUILDS / GAME LIBRARY",
        "Game library",
        "Install a build or apply a delta update",
        |ui| {
            if widgets::primary_button(ui, &theme, "Refresh", Some(Icon::Refresh)).clicked() {
                app.refresh_game_versions();
            }
        },
    );
    ui.add_space(10.0);
    ui.horizontal(|ui| {
        widgets::segmented(
            ui,
            &theme,
            &LibraryFilter::ALL,
            &mut app.ui.library_filter,
            |filter| filter.label().to_string(),
        );
        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
            widgets::search_field(ui, &theme, &mut app.ui.command_query, "Search versions");
        });
    });
    if !app.game_status.is_empty() {
        ui.add_space(5.0);
        ui.label(RichText::new(&app.game_status).font(theme.small()).color(theme.on_surface_variant));
    }
    ui.add_space(10.0);
    if app.game_versions.is_empty() && app.game_status.is_empty() {
        app.refresh_game_versions();
    }

    let installed = launcher_core::launch::find_game_binary(&app.store.settings).ok();
    let versions = app.game_versions.clone();
    if versions.is_empty() {
        egui::Frame::NONE
            .fill(theme.card)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(16))
            .inner_margin(egui::Margin::same(18))
            .show(ui, |ui| {
                widgets::empty_state(ui, &theme, Icon::Grid, "No builds loaded", "Refresh the library to read the published game versions.");
            });
    } else {
        for release in versions {
            let is_installed = installed.as_ref().is_some_and(|_| false);
            egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(15))
                .inner_margin(egui::Margin::same(14))
                .show(ui, |ui| {
                    ui.horizontal(|ui| {
                        let (rect, _) = ui.allocate_exact_size(Vec2::splat(42.0), egui::Sense::hover());
                        ui.painter().rect_filled(rect, 12.0, theme.primary_container);
                        icons::paint(ui.painter(), rect.shrink(11.0), Icon::Box, theme.primary);
                        ui.vertical(|ui| {
                            ui.label(RichText::new(format!("Matticraft {}", release.version)).font(theme.subheading()).strong().color(theme.on_surface));
                            ui.label(RichText::new(&release.name).font(theme.small()).color(theme.on_surface_variant));
                        });
                        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                            if widgets::primary_button(ui, &theme, if is_installed { "Installed" } else { "Install" }, Some(Icon::Download)).clicked() {
                                let version = release.version.clone();
                                app.install_game(&release);
                                if app.game_status.contains("installed") {
                                    app.ui.success("Version installed", format!("Matticraft {version}"));
                                } else {
                                    app.ui.error("Version not installed", app.game_status.clone());
                                }
                            }
                            if let Some(digest) = &release.digest {
                                ui.label(RichText::new(format!("sha256 {}", &digest[..digest.len().min(10)])).font(theme.micro()).color(theme.faint));
                            }
                            ui.label(RichText::new(format!("{} MB", release.size / 1_000_000)).font(theme.micro()).color(theme.on_surface_variant));
                        });
                    });
                });
            ui.add_space(6.0);
        }
    }

    ui.add_space(18.0);
    widgets::section_title(ui, &theme, "Delta update", "The signed manifest can carry a small patch");
    ui.add_space(4.0);
    if let Some(manifest) = &app.update_manifest {
        ui.label(RichText::new(format!("Release {} · {} · {}", manifest.version, manifest.channel, manifest.released_at)).font(theme.small()).color(theme.on_surface));
        if let Some(patch) = manifest.patch_for(launcher_core::platform::Platform::current().name(), launcher_core::platform::arch()) {
            ui.label(RichText::new(format!("{} → {} · {} bytes", patch.from_version, patch.to_version, patch.size)).font(theme.small()).color(theme.on_surface_variant));
            if widgets::secondary_button(ui, &theme, "Apply patch & play", Some(Icon::Play)).clicked() {
                app.apply_update_and_play();
            }
        } else {
            ui.label(RichText::new("No delta for this platform in the current release.").font(theme.small()).color(theme.warning));
        }
    } else {
        ui.label(RichText::new("Check the signed update manifest to see delta availability.").font(theme.small()).color(theme.on_surface_variant));
        if widgets::secondary_button(ui, &theme, "Check updates", Some(Icon::Refresh)).clicked() {
            app.check_updates();
        }
    }
    if !app.update_status.is_empty() {
        ui.label(RichText::new(&app.update_status).font(theme.small()).color(theme.on_surface_variant));
    }
}

fn profiles(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "INSTANCES / PROFILES",
        "Profiles",
        "Separate worlds, mods and renderer settings",
        |ui| {
            if widgets::primary_button(ui, &theme, "New profile", Some(Icon::Plus)).clicked() {
                app.create_profile();
            }
        },
    );
    ui.add_space(10.0);
    ui.horizontal(|ui| {
        widgets::segmented(
            ui,
            &theme,
            &[ProfileLayout::Grid, ProfileLayout::List],
            &mut app.ui.profile_layout,
            |layout| layout.label().to_string(),
        );
        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
            if app.selected.is_some() {
                if widgets::secondary_button(ui, &theme, "Duplicate", Some(Icon::Copy)).clicked() {
                    app.duplicate_profile();
                }
            }
        });
    });
    let profiles = app.store.profiles.clone();
    let selected = app.selected.clone();
    ui.add_space(8.0);
    if app.ui.profile_layout == ProfileLayout::Grid {
        let columns = if ui.available_width() > 900.0 { 3 } else { 2 };
        ui.columns(columns, |columns| {
            let column_count = columns.len();
            for (index, profile) in profiles.iter().enumerate() {
                let column = &mut columns[index % column_count];
                let active = selected.as_deref() == Some(profile.id.as_str());
                let frame = egui::Frame::NONE
                    .fill(if active { theme.primary_container } else { theme.card })
                    .stroke(Stroke::new(1.0, if active { theme.primary } else { theme.outline }))
                    .corner_radius(egui::CornerRadius::same(16))
                    .inner_margin(egui::Margin::same(14));
                frame.show(column, |ui| {
                    ui.horizontal(|ui| {
                        widgets::avatar(ui, &theme, &profile.name, 38.0);
                        ui.vertical(|ui| {
                            ui.label(RichText::new(&profile.name).font(theme.subheading()).strong().color(theme.on_surface));
                            ui.label(RichText::new(profile.display_selection()).font(theme.micro()).color(theme.on_surface_variant));
                        });
                    });
                    ui.add_space(10.0);
                    widgets::key_value(ui, &theme, "Version", &profile.game_version);
                    widgets::key_value(ui, &theme, "Memory", &format!("{} MiB", profile.memory_mb));
                    ui.add_space(8.0);
                    ui.horizontal(|ui| {
                        if widgets::text_button(ui, &theme, if active { "Selected" } else { "Select" }, Some(Icon::Check)).clicked() {
                            app.select_profile(&profile.id);
                        }
                        if icons::button(ui, Icon::Play, 28.0, false, &theme).clicked() {
                            app.select_profile(&profile.id);
                            app.play();
                        }
                    });
                });
                column.add_space(7.0);
            }
        });
    } else {
        for profile in &profiles {
            let active = selected.as_deref() == Some(profile.id.as_str());
            egui::Frame::NONE
                .fill(if active { theme.primary_container } else { theme.card })
                .stroke(Stroke::new(1.0, if active { theme.primary } else { theme.outline }))
                .corner_radius(egui::CornerRadius::same(14))
                .inner_margin(egui::Margin::symmetric(14, 10))
                .show(ui, |ui| {
                    ui.horizontal(|ui| {
                        ui.label(RichText::new(&profile.name).font(theme.body()).strong().color(theme.on_surface));
                        ui.label(RichText::new(&profile.game_version).font(theme.small()).color(theme.on_surface_variant));
                        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                            if ui.small_button(if active { "Selected" } else { "Select" }).clicked() {
                                app.select_profile(&profile.id);
                            }
                        });
                    });
                });
            ui.add_space(5.0);
        }
    }

    ui.add_space(18.0);
    widgets::section_title(ui, &theme, "Profile editor", "Changes are saved through the core profile format");
    ui.add_space(5.0);
    let Some(draft) = app.draft.as_mut() else {
        return;
    };
    let mut changed = false;
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(16))
        .show(ui, |ui| {
            egui::Grid::new("material3-profile-editor")
                .num_columns(2)
                .spacing([20.0, 12.0])
                .show(ui, |ui| {
                    ui.label(RichText::new("Name").font(theme.small()).color(theme.on_surface_variant));
                    changed |= ui.add(egui::TextEdit::singleline(&mut draft.name).desired_width(320.0)).changed();
                    ui.end_row();
                    ui.label(RichText::new("Version").font(theme.small()).color(theme.on_surface_variant));
                    changed |= ui.add(egui::TextEdit::singleline(&mut draft.game_version).desired_width(180.0)).changed();
                    ui.end_row();
                    ui.label(RichText::new("Renderer").font(theme.small()).color(theme.on_surface_variant));
                    egui::ComboBox::from_id_salt("material3-renderer")
                        .selected_text(draft.selection.clone())
                        .width(220.0)
                        .show_ui(ui, |ui| {
                            for preset in launcher_core::profile::PRESETS {
                                changed |= ui.selectable_value(&mut draft.selection, preset.to_string(), *preset).changed();
                            }
                        });
                    ui.end_row();
                    ui.label(RichText::new("Memory").font(theme.small()).color(theme.on_surface_variant));
                    changed |= ui.add(egui::DragValue::new(&mut draft.memory_mb).range(512..=65536).suffix(" MiB")).changed();
                    ui.end_row();
                    ui.label(RichText::new("Window").font(theme.small()).color(theme.on_surface_variant));
                    ui.horizontal(|ui| {
                        changed |= ui.add(egui::DragValue::new(&mut draft.width).range(320..=7680)).changed();
                        ui.label("×");
                        changed |= ui.add(egui::DragValue::new(&mut draft.height).range(240..=4320)).changed();
                        changed |= ui.checkbox(&mut draft.fullscreen, "fullscreen").changed();
                    });
                    ui.end_row();
                });
        });
    if changed {
        app.dirty = true;
    }
    ui.add_space(8.0);
    ui.horizontal(|ui| {
        if widgets::primary_button(ui, &theme, "Save profile", Some(Icon::Check)).clicked() {
            if let Err(error) = app.save_draft() {
                app.ui.error("Profile not saved", error.to_string());
            } else {
                app.ui.success("Profile saved", "The instance is ready.");
            }
        }
        if widgets::secondary_button(ui, &theme, "Open folder", Some(Icon::Folder)).clicked() {
            if let Some(draft) = &app.draft {
                app.open_path(draft.instance_dir.clone());
            }
        }
        if app.store.profiles.len() > 1 && widgets::danger_button(ui, &theme, "Delete profile").clicked() {
            if let Some(id) = app.selected.clone() {
                app.ui.ask("Delete profile?", "The instance folder is kept on disk; only the launcher entry is removed.", ConfirmAction::RemoveProfile(id), true);
            }
        }
    });
}

fn mods(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "EXTENSIONS / MODS",
        "Mods",
        "Installed files, manifests and the mod catalog",
        |ui| {
            if widgets::primary_button(ui, &theme, "Open Mod Studio", Some(Icon::Wrench)).clicked() {
                app.tab = Tab::Studio;
            }
        },
    );
    let profile = app.draft.clone();
    let Some(profile) = profile else {
        widgets::empty_state(ui, &theme, Icon::Layers, "No profile selected", "Create a profile before installing mods.");
        return;
    };
    ui.add_space(8.0);
    egui::Frame::NONE
        .fill(theme.surface_variant)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(14))
        .inner_margin(egui::Margin::same(12))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                icons::paint(ui.painter(), egui::Rect::from_min_size(ui.cursor().min, Vec2::splat(17.0)), Icon::Folder, theme.primary);
                ui.add_space(24.0);
                ui.vertical(|ui| {
                    ui.label(RichText::new("INSTANCE MODS FOLDER").font(theme.micro()).color(theme.primary));
                    ui.monospace(profile.mods_dir.display().to_string());
                });
                ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                    if widgets::secondary_button(ui, &theme, "Open", Some(Icon::External)).clicked() {
                        app.open_path(profile.mods_dir.clone());
                    }
                    if widgets::secondary_button(ui, &theme, "Rescan", Some(Icon::Refresh)).clicked() {
                        app.refresh_profile_views();
                    }
                });
            });
        });
    ui.add_space(12.0);
    let validation = app.mod_validation.clone();
    if let Some(validation) = &validation {
        ui.horizontal(|ui| {
            ui.label(RichText::new(validation.summary()).font(theme.body()).strong().color(if validation.ok() { theme.success } else { theme.error }));
            ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                if widgets::secondary_button(ui, &theme, "Write mods.json", Some(Icon::Check)).clicked() {
                    match launcher_core::mods::build_manifest(&profile.mods_dir)
                        .and_then(|manifest| launcher_core::mods::write_manifest(&profile.mods_dir, &manifest))
                    {
                        Ok(()) => {
                            app.ui.success("Manifest written", "The current files are now the expected set.");
                            app.refresh_profile_views();
                        }
                        Err(error) => app.ui.error("Manifest failed", error.to_string()),
                    }
                }
            });
        });
        ui.add_space(8.0);
        egui::Frame::NONE
            .fill(theme.card)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(14))
            .inner_margin(egui::Margin::same(12))
            .show(ui, |ui| {
                egui::Grid::new("mods-table")
                    .striped(true)
                    .num_columns(3)
                    .spacing([18.0, 7.0])
                    .show(ui, |ui| {
                        ui.label(RichText::new("FILE").font(theme.micro()).color(theme.faint));
                        ui.label(RichText::new("SIZE").font(theme.micro()).color(theme.faint));
                        ui.label(RichText::new("SHA-256").font(theme.micro()).color(theme.faint));
                        ui.end_row();
                        for file in &validation.installed {
                            ui.label(RichText::new(&file.file).font(theme.mono()).color(theme.on_surface));
                            ui.label(RichText::new(format!("{} B", file.size)).font(theme.mono()).color(theme.on_surface_variant));
                            ui.label(RichText::new(&file.sha256[..file.sha256.len().min(16)]).font(theme.mono()).color(theme.on_surface_variant));
                            ui.end_row();
                        }
                    });
            });
        if !validation.ok() {
            ui.add_space(8.0);
            for missing in &validation.missing {
                ui.label(RichText::new(format!("Missing: {missing}")).font(theme.small()).color(theme.error));
            }
            for changed in &validation.mismatched {
                ui.label(RichText::new(format!("Changed: {}", changed.file)).font(theme.small()).color(theme.warning));
            }
        }
    } else {
        widgets::empty_state(ui, &theme, Icon::Layers, "No manifest loaded", "Rescan the instance to validate the mods folder.");
    }

    ui.add_space(18.0);
    widgets::section_title(ui, &theme, "Mod catalog", "Live entries from mods.json in the configured GitHub repository");
    let mut query = app.catalog_query.clone();
    widgets::search_field(ui, &theme, &mut query, "Search the catalog");
    app.catalog_query = query;
    ui.horizontal(|ui| {
        if widgets::secondary_button(ui, &theme, "Refresh catalog", Some(Icon::Refresh)).clicked() {
            app.refresh_catalog();
        }
        if widgets::text_button(ui, &theme, "Catalog settings", Some(Icon::Settings)).clicked() {
            app.tab = Tab::Settings;
                app.ui.settings_section = SettingsSection::Network;
        }
        ui.label(RichText::new(&app.catalog_status).font(theme.small()).color(theme.on_surface_variant));
    });
    ui.add_space(6.0);
    let entries = app
        .catalog
        .as_ref()
        .map(|catalog| {
            let query = app.catalog_query.clone();
            let tag = app.catalog_tag.clone();
            catalog
                .filtered(&query, &tag)
                .into_iter()
                .cloned()
                .collect::<Vec<_>>()
        })
        .unwrap_or_default();
    if entries.is_empty() {
        widgets::empty_state(ui, &theme, Icon::Box, "No catalog loaded", "Refresh the catalog or check the repository URL in Settings → Network.");
    }
    for entry in entries {
        egui::Frame::NONE
            .fill(theme.card)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(13))
            .inner_margin(egui::Margin::same(11))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    let (rect, _) = ui.allocate_exact_size(Vec2::splat(42.0), egui::Sense::hover());
                    ui.painter().rect_filled(rect, 11.0, theme.surface_variant);
                    icons::paint(ui.painter(), rect.shrink(11.0), Icon::Box, theme.primary);
                    ui.vertical(|ui| {
                        ui.horizontal(|ui| {
                            ui.label(RichText::new(&entry.name).font(theme.body()).strong().color(theme.on_surface));
                            widgets::badge(ui, &theme, &entry.version, theme.primary);
                            if entry.featured {
                                widgets::badge(ui, &theme, "FEATURED", theme.warning);
                            }
                        });
                        ui.label(RichText::new(format!("{} · {}", entry.id, entry.author)).font(theme.micro()).color(theme.primary));
                        ui.label(RichText::new(&entry.summary).font(theme.small()).color(theme.on_surface_variant));
                    });
                    ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                        if widgets::secondary_button(ui, &theme, "Install", Some(Icon::Download)).clicked() {
                            app.install_catalog_mod(entry.clone());
                        }
                        if let Some(download) = entry.download_for(launcher_core::platform::Platform::current().name(), launcher_core::platform::arch()) {
                            ui.label(RichText::new(format!("{} KB", download.size / 1000)).font(theme.micro()).color(theme.faint));
                        }
                    });
                });
            });
        ui.add_space(5.0);
    }
}

fn studio_page(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "WORKSHOP / MOD STUDIO",
        "Mod Studio",
        "metadata.matti → MDK → build → .matti → mods/import",
        |ui| {
            if widgets::secondary_button(ui, &theme, "Open studio folder", Some(Icon::Folder)).clicked() {
                app.open_path(app.store.root().join("studio"));
            }
        },
    );
    let project = app.studio.clone();
    if let Some(project) = &project {
        egui::Frame::NONE
            .fill(theme.primary_container)
            .stroke(Stroke::new(1.0, theme.primary))
            .corner_radius(egui::CornerRadius::same(16))
            .inner_margin(egui::Margin::same(14))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    widgets::avatar(ui, &theme, &project.metadata.name, 42.0);
                    ui.vertical(|ui| {
                        ui.label(RichText::new(&project.metadata.name).font(theme.subheading()).strong().color(theme.on_primary_container));
                        ui.label(RichText::new(format!("{} · {} · metadata.matti", project.metadata.id, project.metadata.version)).font(theme.micro()).color(theme.on_primary_container));
                        ui.monospace(project.root.display().to_string());
                    });
                    ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                        if widgets::primary_button(ui, &theme, "Build & pack", Some(Icon::Wrench)).clicked() {
                            app.build_studio();
                        }
                        if widgets::secondary_button(ui, &theme, "Stage import", Some(Icon::Upload)).clicked() {
                            app.stage_studio();
                        }
                    });
                });
            });
    }
    ui.add_space(12.0);
    ui.columns(2, |columns| {
        columns[0].label(RichText::new("NEW MOD PROJECT").font(Theme::display_font(11.0)).color(theme.primary));
        egui::Frame::NONE
            .fill(theme.card)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(14))
            .inner_margin(egui::Margin::same(14))
            .show(&mut columns[0], |ui| {
                for (label, value) in [
                    ("Name", &mut app.studio_name),
                    ("ID", &mut app.studio_id),
                    ("Version", &mut app.studio_version),
                    ("Author", &mut app.studio_author),
                ] {
                    ui.label(RichText::new(label).font(theme.micro()).color(theme.on_surface_variant));
                    ui.add(egui::TextEdit::singleline(value).desired_width(f32::INFINITY));
                }
                ui.label(RichText::new("Description").font(theme.micro()).color(theme.on_surface_variant));
                ui.add(egui::TextEdit::multiline(&mut app.studio_description).desired_rows(3).desired_width(f32::INFINITY));
                ui.add_space(6.0);
                if widgets::primary_button(ui, &theme, "Create project", Some(Icon::Plus)).clicked() {
                    app.create_studio_project();
                }
            });
        columns[1].label(RichText::new("PIPELINE").font(Theme::display_font(11.0)).color(theme.primary));
        egui::Frame::NONE
            .fill(theme.surface_variant)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(14))
            .inner_margin(egui::Margin::same(14))
            .show(&mut columns[1], |ui| {
                for (index, (title, detail)) in [
                    ("metadata.matti", "Mandatory in the project root; mods.json is generated from it."),
                    ("MDK", "Clone mattiasploesch-cpu/matti-ndk or use MATTI_MDK_DIR."),
                    ("Build", "CMake builds the shared object against library/src."),
                    ("Pack", ".matti receives pack, metadata and source sections."),
                    ("Import", "Place the package in mods/import for the new_mod workflow."),
                ]
                .into_iter()
                .enumerate()
                {
                    ui.horizontal(|ui| {
                        let (rect, _) = ui.allocate_exact_size(Vec2::splat(26.0), egui::Sense::hover());
                        ui.painter().circle_filled(rect.center(), 12.0, theme.primary_container);
                        ui.painter().text(rect.center(), egui::Align2::CENTER_CENTER, format!("{}", index + 1), theme.micro(), theme.primary);
                        ui.vertical(|ui| {
                            ui.label(RichText::new(title).font(theme.small()).strong().color(theme.on_surface));
                            ui.label(RichText::new(detail).font(theme.micro()).color(theme.on_surface_variant));
                        });
                    });
                    ui.add_space(4.0);
                }
            });
    });
    if !app.studio_status.is_empty() {
        ui.add_space(10.0);
        ui.label(RichText::new(&app.studio_status).font(theme.small()).color(theme.on_surface_variant));
    }
    if let Some(package) = &app.studio_package {
        ui.label(RichText::new(format!("Package: {}", package.display())).font(theme.mono()).color(theme.primary));
    }
    ui.add_space(14.0);
    widgets::section_title(ui, &theme, "Projects", "Every folder with metadata.matti is a project");
    let projects = launcher_core::studio::list_projects(&app.store.root().join("studio")).unwrap_or_default();
    for project in projects {
        ui.horizontal(|ui| {
            ui.label(RichText::new(format!("{}  {}", project.metadata.name, project.metadata.version)).font(theme.small()).color(theme.on_surface));
            ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                if widgets::text_button(ui, &theme, "Open", Some(Icon::Folder)).clicked() {
                    app.open_studio_project(project.root.clone());
                }
            });
        });
    }
}

fn accounts(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "IDENTITIES / ACCOUNTS",
        "Accounts",
        "Choose the identity used at launch",
        |ui| {
            widgets::avatar(ui, &theme, &app.account_name, 28.0);
        },
    );
    ui.add_space(10.0);
    let accounts = app.store.accounts.clone();
    if accounts.is_empty() {
        widgets::empty_state(ui, &theme, Icon::Users, "No account yet", "Add an offline name below to start the client.");
    }
    for account in &accounts {
        let active = app.store.active_account.as_deref() == Some(account.id.as_str());
        egui::Frame::NONE
            .fill(if active { theme.primary_container } else { theme.card })
            .stroke(Stroke::new(1.0, if active { theme.primary } else { theme.outline }))
            .corner_radius(egui::CornerRadius::same(15))
            .inner_margin(egui::Margin::same(14))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    widgets::avatar(ui, &theme, &account.username, 42.0);
                    ui.vertical(|ui| {
                        ui.label(RichText::new(&account.username).font(theme.subheading()).strong().color(theme.on_surface));
                        ui.label(RichText::new(format!("{:?}", account.kind)).font(theme.micro()).color(theme.on_surface_variant));
                    });
                    ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                        if widgets::danger_button(ui, &theme, "Remove").clicked() {
                            app.ui.ask("Remove account?", "The active identity falls back to the next account.", ConfirmAction::RemoveAccount(account.id.clone()), true);
                        }
                        if widgets::secondary_button(ui, &theme, if active { "Active" } else { "Use" }, Some(Icon::Check)).clicked() {
                            let _ = app.store.set_active_account(&account.id);
                            let _ = app.store.save();
                            app.ui.success("Account selected", &account.username);
                        }
                    });
                });
            });
        ui.add_space(6.0);
    }
    ui.add_space(14.0);
    widgets::section_title(ui, &theme, "Offline account", "Letters, digits and underscore; 3–16 characters");
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(15))
        .inner_margin(egui::Margin::same(14))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.add(egui::TextEdit::singleline(&mut app.account_name).hint_text("Player name").desired_width(220.0));
                if widgets::primary_button(ui, &theme, "Add account", Some(Icon::Plus)).clicked() {
                    app.add_account();
                }
            });
            ui.label(RichText::new("Offline accounts never send an access token. The game derives the offline UUID at launch.").font(theme.small()).color(theme.on_surface_variant));
        });
    ui.add_space(18.0);
    ui.columns(2, |columns| {
        widgets::skin_preview(&mut columns[0], app.skin.as_ref(), &theme, 180.0);
        columns[1].label(RichText::new("SKIN PREVIEW").font(Theme::display_font(11.0)).color(theme.primary));
        columns[1].label(RichText::new("The bundled preview is a local placeholder. A skin service can replace it without changing the account format.").font(theme.small()).color(theme.on_surface_variant));
        columns[1].add_space(10.0);
        widgets::key_value(&mut columns[1], &theme, "UUID", "derived at launch");
        widgets::key_value(&mut columns[1], &theme, "Token", "redacted from debug output");
    });
}

fn servers(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "WORLDS / SERVERS",
        "Servers",
        "Saved worlds, ping and direct connect",
        |ui| {
            if widgets::primary_button(ui, &theme, "Add server", Some(Icon::Plus)).clicked() {
                app.ui.server_dialog_open = true;
            }
        },
    );
    ui.add_space(10.0);
    egui::Frame::NONE
        .fill(theme.surface_variant)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(14))
        .inner_margin(egui::Margin::same(12))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.add(egui::TextEdit::singleline(&mut app.server_address).hint_text("host:port").desired_width(360.0));
                if widgets::primary_button(ui, &theme, "Direct connect", Some(Icon::Play)).clicked() {
                    app.join_server(&app.server_address.clone());
                }
                if !app.server_status.is_empty() {
                    ui.label(RichText::new(&app.server_status).font(theme.small()).color(theme.on_surface_variant));
                }
            });
        });
    ui.add_space(8.0);
    let mut query = app.ui.server_query.clone();
    widgets::search_field(ui, &theme, &mut query, "Search servers");
    app.ui.server_query = query;
    ui.add_space(8.0);
    let filter = app.ui.server_query.to_lowercase();
    for (index, server) in SERVERS.iter().enumerate() {
        if !filter.is_empty()
            && !server.name.to_lowercase().contains(&filter)
            && !server.address.to_lowercase().contains(&filter)
        {
            continue;
        }
        let selected = app.ui.selected_server == index;
        egui::Frame::NONE
            .fill(if selected { theme.primary_container } else { theme.card })
            .stroke(Stroke::new(1.0, if selected { theme.primary } else { theme.outline }))
            .corner_radius(egui::CornerRadius::same(14))
            .inner_margin(egui::Margin::symmetric(14, 11))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    let (rect, _) = ui.allocate_exact_size(Vec2::splat(38.0), egui::Sense::hover());
                    ui.painter().rect_filled(rect, 11.0, theme.surface_variant);
                    icons::paint(ui.painter(), rect.shrink(10.0), Icon::Globe, theme.primary);
                    ui.vertical(|ui| {
                        ui.horizontal(|ui| {
                            ui.label(RichText::new(server.name).font(theme.body()).strong().color(theme.on_surface));
                            widgets::badge(ui, &theme, server.region, data::server_region_color(server.region));
                            if server.favorite {
                                ui.label(RichText::new("★").font(theme.small()).color(theme.warning));
                            }
                        });
                        ui.label(RichText::new(server.motd).font(theme.small()).color(theme.on_surface_variant));
                        ui.label(RichText::new(server.address).font(theme.mono()).color(theme.primary));
                    });
                    ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                        widgets::ping_bars(ui, &theme, server.ping);
                        ui.label(RichText::new(format!("{}/{}", server.players, server.max)).font(theme.small()).color(theme.on_surface_variant));
                        if widgets::primary_button(ui, &theme, "Join", Some(Icon::Play)).clicked() {
                            app.ui.selected_server = index;
                            connect_server(app, server.address);
                        }
                    });
                });
            });
        ui.add_space(5.0);
    }
    if app.ui.server_dialog_open {
        let mut open = true;
        egui::Window::new("add-server")
            .title_bar(false)
            .collapsible(false)
            .resizable(false)
            .anchor(egui::Align2::CENTER_CENTER, [0.0, 0.0])
            .fixed_size([420.0, 190.0])
            .open(&mut open)
            .frame(super::theme::modal_frame(&theme))
            .show(ui.ctx(), |ui| {
                ui.label(RichText::new("Add server").font(theme.subheading()).strong().color(theme.on_surface));
                ui.add(egui::TextEdit::singleline(&mut app.ui.server_form_name).hint_text("Name"));
                ui.add(egui::TextEdit::singleline(&mut app.ui.server_form_address).hint_text("host:port"));
                ui.add_space(8.0);
                if widgets::primary_button(ui, &theme, "Save server", Some(Icon::Check)).clicked() {
                    if !app.ui.server_form_address.trim().is_empty() {
                        app.ui.success("Server saved", "Use Join on the server entry to pass --server to the game.");
                    }
                    app.ui.server_dialog_open = false;
                }
            });
        app.ui.server_dialog_open = open;
    }
}

fn connect_server(app: &mut LauncherApp, address: &str) {
    app.join_server(address);
}


fn cosmetics(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "PROFILE / COSMETICS",
        "Cosmetics",
        "Hats, capes and emotes for your next session",
        |ui| {
            widgets::skin_preview(ui, app.skin.as_ref(), &theme, 52.0);
        },
    );
    ui.add_space(8.0);
    widgets::segmented(
        ui,
        &theme,
        &CosmeticCategory::ALL,
        &mut app.ui.cosmetic_category,
        |category| category.label().to_string(),
    );
    let mut query = app.ui.cosmetic_query.clone();
    widgets::search_field(ui, &theme, &mut query, "Search cosmetics");
    app.ui.cosmetic_query = query;
    ui.add_space(10.0);
    let filter = app.ui.cosmetic_query.to_lowercase();
    let items: Vec<_> = COSMETICS
        .iter()
        .enumerate()
        .filter(|(_, item)| app.ui.cosmetic_category.matches(item.category))
        .filter(|(_, item)| filter.is_empty() || item.name.to_lowercase().contains(&filter))
        .collect();
    if items.is_empty() {
        widgets::empty_state(ui, &theme, Icon::Sparkles, "Nothing matches", "Try another category or search term.");
    }
    ui.columns(3, |columns| {
        for (index, item) in items {
            let column = &mut columns[index % 3];
            let selected = app.ui.selected_cosmetic == Some(index);
            egui::Frame::NONE
                .fill(if selected { theme.primary_container } else { theme.card })
                .stroke(Stroke::new(1.0, if selected { theme.primary } else { theme.outline }))
                .corner_radius(egui::CornerRadius::same(15))
                .inner_margin(egui::Margin::same(12))
                .show(column, |ui| {
                    ui.horizontal(|ui| {
                        let (rect, _) = ui.allocate_exact_size(Vec2::splat(42.0), egui::Sense::hover());
                        ui.painter().circle_filled(rect.center(), 20.0, Theme::alpha(data::rarity_color(item.rarity), 32));
                        ui.painter().text(rect.center(), egui::Align2::CENTER_CENTER, item.icon, FontId::proportional(24.0), data::rarity_color(item.rarity));
                        ui.vertical(|ui| {
                            ui.label(RichText::new(item.name).font(theme.body()).strong().color(theme.on_surface));
                            ui.label(RichText::new(item.rarity.to_uppercase()).font(theme.micro()).color(data::rarity_color(item.rarity)));
                        });
                    });
                    ui.label(RichText::new(item.description).font(theme.small()).color(theme.on_surface_variant));
                    ui.horizontal(|ui| {
                        widgets::badge(ui, &theme, if item.owned { "OWNED" } else { "LOCKED" }, if item.owned { theme.success } else { theme.on_surface_variant });
                        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                            if widgets::text_button(ui, &theme, if item.owned { "Equip" } else { "Preview" }, Some(Icon::Sparkles)).clicked() {
                                app.ui.selected_cosmetic = Some(index);
                                app.ui.info(item.name, if item.owned { "Equipped for this session." } else { "Preview only; this item is not owned." });
                            }
                        });
                    });
                });
            column.add_space(7.0);
        }
    });
    ui.add_space(18.0);
    widgets::section_title(ui, &theme, "Equipped", "The cosmetic preferences are stored per account");
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(14))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                for slot in ["HAT", "CAPE", "WINGS", "AURA"] {
                    let value = match slot {
                        "HAT" => "Sprout",
                        "CAPE" => "Signal Cape",
                        "WINGS" => "Glass Wings",
                        _ => "Spark Aura",
                    };
                    ui.vertical(|ui| {
                        ui.label(RichText::new(slot).font(theme.micro()).color(theme.faint));
                        ui.label(RichText::new(value).font(theme.small()).color(theme.on_surface));
                    });
                    ui.add_space(20.0);
                }
            });
        });
}

fn news(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "FEED / NEWS",
        "News & updates",
        "Release notes and development updates",
        |ui| {
            if widgets::secondary_button(ui, &theme, "Changelog", Some(Icon::Clock)).clicked() {
                app.ui.info("Changelog", data::CHANGELOG[0].changes.join(" · "));
            }
        },
    );
    ui.add_space(8.0);
    widgets::segmented(
        ui,
        &theme,
        &NewsCategory::ALL,
        &mut app.ui.news_category,
        |category| category.label().to_string(),
    );
    ui.add_space(10.0);
    let items: Vec<_> = NEWS
        .iter()
        .enumerate()
        .filter(|(_, item)| app.ui.news_category.matches(item.category))
        .collect();
    ui.columns(2, |columns| {
        columns[0].label(RichText::new("LATEST").font(Theme::display_font(11.0)).color(theme.primary));
        for (index, item) in &items {
            let selected = app.ui.selected_news == *index;
            let response = egui::Frame::NONE
                .fill(if selected { theme.primary_container } else { theme.card })
                .stroke(Stroke::new(1.0, if selected { theme.primary } else { theme.outline }))
                .corner_radius(egui::CornerRadius::same(13))
                .inner_margin(egui::Margin::same(11))
                .show(&mut columns[0], |ui| {
                    ui.horizontal(|ui| {
                        widgets::badge(ui, &theme, data::category_label(item.category), theme.secondary);
                        ui.label(RichText::new(item.date).font(theme.micro()).color(theme.faint));
                    });
                    ui.label(RichText::new(item.title).font(theme.body()).strong().color(theme.on_surface));
                    ui.label(RichText::new(item.summary).font(theme.small()).color(theme.on_surface_variant));
                })
                .response
                .interact(egui::Sense::click());
            if response.clicked() {
                app.ui.selected_news = *index;
            }
            columns[0].add_space(6.0);
        }
        if let Some((_, item)) = items.get(app.ui.selected_news.min(items.len().saturating_sub(1))) {
            columns[1].label(RichText::new("READ").font(Theme::display_font(11.0)).color(theme.primary));
            egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(16))
                .inner_margin(egui::Margin::same(18))
                .show(&mut columns[1], |ui| {
                    ui.horizontal(|ui| {
                        widgets::badge(ui, &theme, data::category_label(item.category), theme.secondary);
                        ui.label(RichText::new(item.date).font(theme.micro()).color(theme.faint));
                        ui.label(RichText::new(format!("{} min", item.read_minutes)).font(theme.micro()).color(theme.faint));
                    });
                    ui.add_space(7.0);
                    ui.label(RichText::new(item.title).font(FontId::proportional(24.0)).strong().color(theme.on_surface));
                    ui.label(RichText::new(item.summary).font(theme.body()).color(theme.primary));
                    ui.add_space(10.0);
                    ui.label(RichText::new(item.body).font(theme.body()).color(theme.on_surface_variant));
                    ui.add_space(14.0);
                    ui.label(RichText::new(format!("By {}", item.author)).font(theme.micro()).color(theme.faint));
                });
        }
    });
}

fn downloads(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "TRANSFERS / DOWNLOADS",
        "Downloads",
        "Queue, install and keep builds up to date",
        |ui| {
            if widgets::primary_button(ui, &theme, "Refresh", Some(Icon::Refresh)).clicked() {
                app.refresh_game_versions();
            }
        },
    );
    ui.add_space(10.0);
    ui.columns(2, |columns| {
        columns[0].label(RichText::new("ACTIVE GAME DOWNLOADS").font(Theme::display_font(11.0)).color(theme.primary));
        if app.ui.downloads.is_empty() {
            widgets::empty_state(&mut columns[0], &theme, Icon::Download, "No active downloads", "Install a version from the library to start a transfer.");
        } else {
            for item in &app.ui.downloads {
                egui::Frame::NONE
                    .fill(theme.card)
                    .stroke(Stroke::new(1.0, theme.outline))
                    .corner_radius(egui::CornerRadius::same(14))
                    .inner_margin(egui::Margin::same(12))
                    .show(&mut columns[0], |ui| {
                        ui.horizontal(|ui| {
                            ui.label(RichText::new(&item.version).font(theme.body()).strong().color(theme.on_surface));
                            ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                                widgets::badge(ui, &theme, item.state.label(), theme.primary);
                            });
                        });
                        widgets::progress(ui, &theme, item.progress, theme.primary);
                        ui.label(RichText::new(format!("{} / {} MB", item.bytes / 1_000_000, item.total / 1_000_000)).font(theme.micro()).color(theme.faint));
                    });
                columns[0].add_space(6.0);
            }
        }
        columns[0].add_space(12.0);
        if widgets::secondary_button(&mut columns[0], &theme, "Clear finished", Some(Icon::Close)).clicked() {
            app.ui.clear_finished_downloads();
        }
        columns[1].label(RichText::new("AVAILABLE BUILDS").font(Theme::display_font(11.0)).color(theme.primary));
        let releases = app.game_versions.iter().take(8).cloned().collect::<Vec<_>>();
        for release in releases {
            egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(13))
                .inner_margin(egui::Margin::symmetric(11, 9))
                .show(&mut columns[1], |ui| {
                    ui.horizontal(|ui| {
                        ui.label(RichText::new(format!("Matticraft {}", release.version)).font(theme.small()).strong().color(theme.on_surface));
                        ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                            if ui.small_button("Install").clicked() {
                                app.install_game(&release);
                            }
                        });
                    });
                    ui.label(RichText::new(format!("{} MB", release.size / 1_000_000)).font(theme.micro()).color(theme.faint));
                });
            columns[1].add_space(5.0);
        }
    });
}

fn logs(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "DIAGNOSTICS / LOGS",
        "Logs & crashes",
        "Read latest.log and inspect crash reports",
        |ui| {
            if widgets::primary_button(ui, &theme, "Refresh", Some(Icon::Refresh)).clicked() {
                app.refresh_profile_views();
            }
        },
    );
    ui.add_space(10.0);
    let mut query = app.ui.logs_query.clone();
    widgets::search_field(ui, &theme, &mut query, "Filter log lines");
    app.ui.logs_query = query;
    widgets::segmented(ui, &theme, &LogLevel::ALL, &mut app.ui.log_level, |level| level.label().to_string());
    ui.add_space(8.0);
    ui.columns(2, |columns| {
        columns[0].label(RichText::new("LATEST.LOG").font(Theme::display_font(11.0)).color(theme.primary));
        egui::Frame::NONE
            .fill(theme.card)
            .stroke(Stroke::new(1.0, theme.outline))
            .corner_radius(egui::CornerRadius::same(14))
            .inner_margin(egui::Margin::same(10))
            .show(&mut columns[0], |ui| {
                egui::ScrollArea::vertical().max_height(360.0).show(ui, |ui| {
                    let filter = app.ui.logs_query.to_lowercase();
                    let mut shown = 0;
                    for line in app.log_lines.iter().rev() {
                        if !app.ui.log_level.matches(line) || (!filter.is_empty() && !line.to_lowercase().contains(&filter)) {
                            continue;
                        }
                        let color = if line.contains("ERROR") {
                            theme.error
                        } else if line.contains("WARN") {
                            theme.warning
                        } else {
                            theme.on_surface_variant
                        };
                        ui.label(RichText::new(line).font(theme.mono()).color(color));
                        shown += 1;
                        if shown > 400 {
                            break;
                        }
                    }
                    if shown == 0 {
                        ui.label(RichText::new("(no matching lines)").font(theme.small()).color(theme.faint));
                    }
                });
            });
        columns[1].label(RichText::new("CRASH REPORTS").font(Theme::display_font(11.0)).color(theme.primary));
        let reports = app.crash_reports.clone();
        for report in &reports {
            let name = report.file_name().map(|name| name.to_string_lossy().to_string()).unwrap_or_default();
            let active = app.selected_crash.as_ref() == Some(report);
            if columns[1].selectable_label(active, name).clicked() {
                app.show_crash(report.clone());
            }
        }
        if app.crash_reports.is_empty() {
            widgets::empty_state(&mut columns[1], &theme, Icon::Shield, "No crash reports", "A panic writes a report next to the instance.");
        }
        if !app.crash_text.is_empty() {
            columns[1].add_space(8.0);
            egui::Frame::NONE
                .fill(theme.card)
                .stroke(Stroke::new(1.0, theme.outline))
                .corner_radius(egui::CornerRadius::same(12))
                .inner_margin(egui::Margin::same(9))
                .show(&mut columns[1], |ui| {
                    egui::ScrollArea::vertical().max_height(180.0).show(ui, |ui| {
                        ui.label(RichText::new(&app.crash_text).font(theme.mono()).color(theme.on_surface_variant));
                    });
                });
        }
        columns[1].add_space(6.0);
        let mut consent = app.store.settings.upload_crash_reports;
        if columns[1].checkbox(&mut consent, "Allow crash report upload (opt-in)").changed() {
            app.store.settings.upload_crash_reports = consent;
            let _ = app.store.save();
        }
        if columns[1].add_enabled(consent && app.selected_crash.is_some(), egui::Button::new("Upload selected")).clicked() {
            app.upload_crash();
        }
    });
}

fn settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "SYSTEM / SETTINGS",
        "Settings",
        "Appearance, game, network and privacy",
        |ui| {
            if widgets::primary_button(ui, &theme, "Save", Some(Icon::Check)).clicked() {
                if let Err(error) = app.save_settings() {
                    app.ui.error("Settings not saved", error.to_string());
                } else {
                    app.ui.success("Settings saved", "The launcher configuration is up to date.");
                }
            }
        },
    );
    ui.add_space(8.0);
    let mut section = app.ui.settings_section;
    widgets::segmented(ui, &theme, &SettingsSection::ALL, &mut section, |section| section.label().to_string());
    app.ui.settings_section = section;
    ui.add_space(6.0);
    ui.label(RichText::new(section.description()).font(theme.small()).color(theme.on_surface_variant));
    ui.add_space(10.0);
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(16))
        .inner_margin(egui::Margin::same(18))
        .show(ui, |ui| match section {
            SettingsSection::Appearance => appearance(app, ui),
            SettingsSection::Game => game_settings(app, ui),
            SettingsSection::Java => java_settings(app, ui),
            SettingsSection::Network => network_settings(app, ui),
            SettingsSection::Privacy => privacy_settings(app, ui),
            SettingsSection::About => about_settings(app, ui),
        });
}

fn appearance(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::section_title(ui, &theme, "Palette", "Material 3 roles from the client");
    ui.add_space(6.0);
    for preset in ThemePreset::ALL {
        let active = app.theme.preset == preset;
        let response = egui::Frame::NONE
            .fill(if active { theme.primary_container } else { theme.surface_variant })
            .stroke(Stroke::new(1.0, if active { theme.primary } else { theme.outline }))
            .corner_radius(egui::CornerRadius::same(12))
            .inner_margin(egui::Margin::same(10))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    let (rect, _) = ui.allocate_exact_size(Vec2::splat(22.0), egui::Sense::hover());
                    ui.painter().circle_filled(rect.center(), 10.0, preset.primary());
                    ui.vertical(|ui| {
                        ui.label(RichText::new(preset.label()).font(theme.small()).strong().color(theme.on_surface));
                        ui.label(RichText::new(preset.description()).font(theme.micro()).color(theme.on_surface_variant));
                    });
                    ui.with_layout(Layout::right_to_left(Align::Center), |ui| {
                        if active {
                            ui.label(RichText::new("ACTIVE").font(theme.micro()).color(theme.primary));
                        }
                    });
                });
            })
            .response
            .interact(egui::Sense::click());
        if response.clicked() {
            app.theme = Theme::new(preset);
            app.ui.accent_index = 0;
        }
        ui.add_space(5.0);
    }
    ui.add_space(10.0);
    widgets::toggle_row(ui, &theme, "Background artwork", "Show the vendored Minecraft panorama on Home.", &mut app.ui.show_background);
    widgets::toggle_row(ui, &theme, "Compact density", "Reduce list spacing for smaller screens.", &mut app.ui.compact);
    widgets::toggle_row(ui, &theme, "Animations", "Fade between pages and animate toasts.", &mut app.ui.animations);
    widgets::toggle_row(ui, &theme, "Reduce motion", "Keep transitions instant even when animations are enabled.", &mut app.ui.reduce_motion);
    widgets::divider(ui, &theme);
    ui.horizontal(|ui| {
        ui.label(RichText::new("Background dim").font(theme.small()).color(theme.on_surface_variant));
        ui.add(egui::Slider::new(&mut app.ui.background_dim, 0.0..=0.95).show_value(false));
    });
}

fn game_settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::section_title(ui, &theme, "Game binary", "The selected build is used for every profile unless overridden");
    ui.horizontal(|ui| {
        ui.add(egui::TextEdit::singleline(&mut app.game_binary_text).hint_text("auto-detect runClient/matticraft").desired_width(430.0));
        if widgets::secondary_button(ui, &theme, "Clear", Some(Icon::Close)).clicked() {
            app.game_binary_text.clear();
        }
    });
    ui.label(RichText::new("Channel").font(theme.small()).color(theme.on_surface_variant));
    ui.add(egui::TextEdit::singleline(&mut app.store.settings.channel).desired_width(180.0));
    widgets::toggle_row(ui, &theme, "Close launcher after launch", "The game keeps running when the launcher exits.", &mut app.store.settings.close_on_launch);
    ui.add_space(8.0);
    if widgets::secondary_button(ui, &theme, "Open data folder", Some(Icon::Folder)).clicked() {
        app.open_path(app.store.root().to_path_buf());
    }
}

fn java_settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    let Some(draft) = app.draft.as_mut() else {
        ui.label(RichText::new("No profile selected.").font(theme.body()).color(theme.on_surface_variant));
        return;
    };
    widgets::section_title(ui, &theme, "Defaults for this profile", "The values are written to profile.json");
    ui.horizontal(|ui| {
        ui.label(RichText::new("Memory").font(theme.small()).color(theme.on_surface_variant));
        ui.add(egui::DragValue::new(&mut draft.memory_mb).range(512..=65536).suffix(" MiB"));
    });
    ui.horizontal(|ui| {
        ui.label(RichText::new("Resolution").font(theme.small()).color(theme.on_surface_variant));
        ui.add(egui::DragValue::new(&mut draft.width).range(320..=7680));
        ui.label("×");
        ui.add(egui::DragValue::new(&mut draft.height).range(240..=4320));
        ui.checkbox(&mut draft.fullscreen, "Fullscreen");
    });
    ui.label(RichText::new("Renderer").font(theme.small()).color(theme.on_surface_variant));
    egui::ComboBox::from_id_salt("java-renderer")
        .selected_text(draft.selection.clone())
        .show_ui(ui, |ui| {
            for preset in launcher_core::profile::PRESETS {
                ui.selectable_value(&mut draft.selection, preset.to_string(), *preset);
            }
        });
    ui.label(RichText::new("Extra arguments").font(theme.small()).color(theme.on_surface_variant));
    let mut args = draft.extra_args.join(" ");
    if ui.add(egui::TextEdit::singleline(&mut args).hint_text("--server host:port").desired_width(430.0)).changed() {
        draft.extra_args = args.split_whitespace().map(ToString::to_string).collect();
    }
}

fn network_settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::section_title(ui, &theme, "Update manifest", "A signed JSON document with artifacts and optional deltas");
    ui.add(egui::TextEdit::singleline(&mut app.update_url_text).desired_width(f32::INFINITY));
    ui.label(RichText::new("The default URL points at the latest GitHub release. A local path is accepted for development.").font(theme.small()).color(theme.on_surface_variant));
    ui.horizontal(|ui| {
        if widgets::secondary_button(ui, &theme, "Check now", Some(Icon::Refresh)).clicked() {
            app.check_updates();
        }
        ui.label(RichText::new(&app.update_status).font(theme.small()).color(theme.on_surface_variant));
    });
    if let Some(manifest) = &app.update_manifest {
        ui.add_space(8.0);
        widgets::key_value(ui, &theme, "Release", &manifest.version);
        widgets::key_value(ui, &theme, "Channel", &manifest.channel);
        widgets::key_value(ui, &theme, "Published", &manifest.released_at);
    }
    widgets::divider(ui, &theme);
    widgets::section_title(ui, &theme, "Mod catalog", "GitHub repository that contains mods.json");
    ui.add(egui::TextEdit::singleline(&mut app.store.settings.catalog_repo).desired_width(f32::INFINITY));
    ui.label(RichText::new("Default: mattiasploesch-cpu/LibMatti (reads mods/mods.json)").font(theme.small()).color(theme.on_surface_variant));
    widgets::section_title(ui, &theme, "MDK repository", "Used by Mod Studio to create a project");
    ui.add(egui::TextEdit::singleline(&mut app.store.settings.mdk_repo).desired_width(f32::INFINITY));
    widgets::divider(ui, &theme);
    widgets::section_title(ui, &theme, "Crash endpoint", "Only used with the privacy opt-in");
    ui.add(egui::TextEdit::singleline(&mut app.crash_url_text).desired_width(f32::INFINITY));
}

fn privacy_settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::section_title(ui, &theme, "Diagnostics", "Local by default");
    widgets::toggle_row(ui, &theme, "Upload crash reports", "Send a report only after pressing Upload.", &mut app.store.settings.upload_crash_reports);
    ui.label(RichText::new("Crash reports contain the launcher version, game version, profile name, renderer selection and platform. Access tokens are never included.").font(theme.small()).color(theme.on_surface_variant));
    ui.add_space(10.0);
    widgets::key_value(ui, &theme, "Data root", &app.store.root().display().to_string());
    if widgets::danger_button(ui, &theme, "Reset launcher preferences").clicked() {
        app.ui.ask("Reset preferences?", "Theme, density and filters return to their defaults. Profiles and accounts are kept.", ConfirmAction::ResetPreferences, false);
    }
}

fn about_settings(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::section_title(ui, &theme, "Build", "Versions and runtime information");
    widgets::key_value(ui, &theme, "Launcher", env!("CARGO_PKG_VERSION"));
    widgets::key_value(ui, &theme, "Platform", &format!("{} {}", launcher_core::platform::Platform::current().name(), launcher_core::platform::arch()));
    widgets::key_value(ui, &theme, "Game binary", app.store.settings.game_binary.as_ref().map(|path| path.display().to_string()).unwrap_or_else(|| "auto".into()).as_str());
    widgets::key_value(ui, &theme, "Profiles", &app.store.profiles.len().to_string());
    widgets::key_value(ui, &theme, "Accounts", &app.store.accounts.len().to_string());
    ui.add_space(8.0);
    if widgets::secondary_button(ui, &theme, "Open the About page", Some(Icon::Info)).clicked() {
        app.tab = Tab::About;
    }
}

fn about(app: &mut LauncherApp, ui: &mut egui::Ui) {
    let theme = app.theme;
    widgets::page_header(
        ui,
        &theme,
        "MATTICRAFT / ABOUT",
        "About Matticraft",
        "Build information, credits and licenses",
        |ui| {
            widgets::badge(ui, &theme, "LGPL-3.0", theme.primary);
        },
    );
    ui.add_space(12.0);
    egui::Frame::NONE
        .fill(theme.card)
        .stroke(Stroke::new(1.0, theme.outline))
        .corner_radius(egui::CornerRadius::same(18))
        .inner_margin(egui::Margin::same(20))
        .show(ui, |ui| {
            ui.label(RichText::new("A from-scratch C port of the Minecraft client, wrapped in a Rust launcher.").font(FontId::proportional(20.0)).strong().color(theme.on_surface));
            ui.label(RichText::new("The launcher owns profiles, downloads, mod validation and diagnostics. The game owns the world and the renderer.").font(theme.body()).color(theme.on_surface_variant));
            ui.add_space(12.0);
            ui.horizontal(|ui| {
                widgets::stat(ui, &theme, env!("CARGO_PKG_VERSION"), "launcher", Icon::Info);
                widgets::stat(ui, &theme, launcher_core::platform::arch(), "architecture", Icon::Monitor);
                widgets::stat(ui, &theme, &app.store.profiles.len().to_string(), "profiles", Icon::Layers);
            });
        });
    ui.add_space(16.0);
    ui.columns(2, |columns| {
        columns[0].label(RichText::new("SHORTCUTS").font(Theme::display_font(11.0)).color(theme.primary));
        for shortcut in SHORTCUTS.iter().take(10) {
            columns[0].horizontal(|ui| {
                widgets::badge(ui, &theme, shortcut.keys, theme.secondary);
                ui.label(RichText::new(shortcut.action).font(theme.small()).color(theme.on_surface_variant));
            });
        }
        columns[1].label(RichText::new("FREQUENT QUESTIONS").font(Theme::display_font(11.0)).color(theme.primary));
        for faq in FAQ.iter().take(6) {
            columns[1].label(RichText::new(faq.question).font(theme.small()).strong().color(theme.on_surface));
            columns[1].label(RichText::new(faq.answer).font(theme.micro()).color(theme.on_surface_variant));
            columns[1].add_space(5.0);
        }
    });
    ui.add_space(16.0);
    widgets::section_title(ui, &theme, "Changelog", "Recent launcher milestones");
    for entry in data::CHANGELOG.iter().take(6) {
        ui.horizontal(|ui| {
            widgets::badge(ui, &theme, entry.version, theme.primary);
            ui.label(RichText::new(entry.date).font(theme.micro()).color(theme.faint));
            ui.label(RichText::new(entry.changes.join(" · ")).font(theme.small()).color(theme.on_surface_variant));
        });
    }
}
