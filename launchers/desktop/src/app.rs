//! The egui frontend. It renders [`launcher_core`] state only; every decision
//! (profile format, selection, mods, updates) stays in the core.

use std::path::{Path, PathBuf};

use eframe::egui;
use egui::{Color32, RichText};
use launcher_core::account::Account;
use launcher_core::launch::{find_game_binary, preflight, LaunchPlan};
use launcher_core::logs;
use launcher_core::mods::{self, ModValidation};
use launcher_core::platform;
use launcher_core::profile::{Profile, PRESETS};
use launcher_core::store::Store;
use launcher_core::update::UpdateManifest;
use launcher_core::{Error, Result};

use crate::LauncherOptions;

const ACCENT: Color32 = Color32::from_rgb(74, 222, 128);
const ACCENT_DARK: Color32 = Color32::from_rgb(34, 120, 70);
const MUTED: Color32 = Color32::from_rgb(150, 160, 170);
const BG: Color32 = Color32::from_rgb(15, 17, 21);
const PANEL: Color32 = Color32::from_rgb(22, 25, 31);
const CARD: Color32 = Color32::from_rgb(30, 34, 42);

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum Tab {
    Profiles,
    Mods,
    Logs,
    Settings,
}

pub fn run_native(options: LauncherOptions) -> eframe::Result<()> {
    let mut native = eframe::NativeOptions::default();
    native.viewport = egui::ViewportBuilder::default()
        .with_inner_size([1120.0, 760.0])
        .with_min_inner_size([900.0, 600.0])
        .with_title("Matticraft Launcher");
    eframe::run_native(
        "Matticraft Launcher",
        native,
        Box::new(move |cc| Ok(Box::new(LauncherApp::new(cc, options)))),
    )
}

struct LauncherApp {
    store: Store,
    open_error: Option<String>,
    selected: Option<String>,
    draft: Option<Profile>,
    dirty: bool,
    tab: Tab,
    account_name: String,
    status: String,
    log_lines: Vec<String>,
    crash_reports: Vec<PathBuf>,
    selected_crash: Option<PathBuf>,
    crash_text: String,
    mod_validation: Option<ModValidation>,
    game_binary_text: String,
    update_url_text: String,
    crash_url_text: String,
    update_status: String,
    update_manifest: Option<UpdateManifest>,
    child: Option<std::process::Child>,
    child_pid: Option<u32>,
    frames: u64,
    max_frames: Option<u64>,
}

impl LauncherApp {
    fn new(cc: &eframe::CreationContext<'_>, options: LauncherOptions) -> Self {
        install_style(&cc.egui_ctx);
        let max_frames = options
            .frames
            .or_else(|| options.one_frame.then_some(2));
        let (store, open_error) = match crate::open_store(&options) {
            Ok(store) => (store, None),
            Err(error) => {
                let root = options
                    .root
                    .clone()
                    .or_else(|| Store::default_root().ok())
                    .unwrap_or_else(|| PathBuf::from("."));
                (Store::empty(root), Some(error.to_string()))
            }
        };

        let selected = store
            .active
            .clone()
            .or_else(|| store.profiles.first().map(|profile| profile.id.clone()));
        let draft = selected
            .as_deref()
            .and_then(|id| store.profile(id))
            .cloned();
        let game_binary_text = store
            .settings
            .game_binary
            .as_ref()
            .map(|path| path.display().to_string())
            .unwrap_or_default();
        let update_url_text = store.settings.update_manifest_url.clone().unwrap_or_default();
        let crash_url_text = store.settings.crash_upload_url.clone().unwrap_or_default();

        let crash_dir = store.root().join(logs::CRASH_DIR);
        if let Some(profile) = draft.as_ref() {
            logs::install_panic_hook(crash_dir, crate::crash_context(&store, Some(profile)));
        }

        let mut app = LauncherApp {
            store,
            open_error,
            selected,
            draft,
            dirty: false,
            tab: Tab::Profiles,
            account_name: String::new(),
            status: String::new(),
            log_lines: Vec::new(),
            crash_reports: Vec::new(),
            selected_crash: None,
            crash_text: String::new(),
            mod_validation: None,
            game_binary_text,
            update_url_text,
            crash_url_text,
            update_status: String::new(),
            update_manifest: None,
            child: None,
            child_pid: None,
            frames: 0,
            max_frames,
        };
        app.refresh_profile_views();
        app
    }

    fn select_profile(&mut self, id: &str) {
        if let Some(profile) = self.store.profile(id).cloned() {
            self.selected = Some(id.to_string());
            self.draft = Some(profile);
            self.dirty = false;
            self.update_status.clear();
            self.update_manifest = None;
            self.refresh_profile_views();
        }
    }

    fn refresh_profile_views(&mut self) {
        let Some(profile) = self.draft.clone() else {
            self.mod_validation = None;
            self.log_lines.clear();
            self.crash_reports.clear();
            return;
        };
        self.mod_validation = mods::validate(&profile.mods_dir).ok();
        self.log_lines = logs::tail(
            &profile.logs_dir.join(logs::LOG_FILE),
            250,
        )
        .unwrap_or_default();
        let crash_dir = logs::crash_reports_dir(&profile.instance_dir);
        self.crash_reports = logs::list_crash_reports(&crash_dir).unwrap_or_default();
        if self.selected_crash.is_none() {
            if let Some(first) = self.crash_reports.first() {
                self.show_crash(first.clone());
            }
        }
    }

    fn show_crash(&mut self, path: PathBuf) {
        self.crash_text = std::fs::read_to_string(&path).unwrap_or_else(|error| error.to_string());
        self.selected_crash = Some(path);
    }

    fn save_draft(&mut self) -> Result<()> {
        let Some(draft) = self.draft.clone() else {
            return Err(Error::Invalid("no profile is selected".into()));
        };
        draft.validate()?;
        preflight(&draft)?;
        draft.save()?;
        if let Some(existing) = self
            .store
            .profiles
            .iter_mut()
            .find(|profile| profile.id == draft.id)
        {
            *existing = draft.clone();
        } else {
            self.store.profiles.push(draft.clone());
        }
        self.store.save()?;
        self.selected = Some(draft.id.clone());
        self.draft = Some(draft);
        self.dirty = false;
        self.status = "Profile saved".into();
        self.refresh_profile_views();
        Ok(())
    }

    fn save_settings(&mut self) -> Result<()> {
        self.store.settings.game_binary = non_empty_path(&self.game_binary_text);
        self.store.settings.update_manifest_url =
            non_empty(&self.update_url_text).map(ToString::to_string);
        self.store.settings.crash_upload_url =
            non_empty(&self.crash_url_text).map(ToString::to_string);
        self.store.save()?;
        self.status = "Settings saved".into();
        Ok(())
    }

    fn play(&mut self) {
        if let Err(error) = self.save_draft().and_then(|_| self.save_settings()) {
            self.status = format!("Cannot start: {error}");
            return;
        }
        let Some(profile) = self.draft.clone() else {
            return;
        };
        if let Err(error) = preflight(&profile) {
            self.status = error.to_string();
            return;
        }
        let account = self.store.active_account().cloned();
        match LaunchPlan::prepare(&profile, &self.store.settings, account.as_ref(), &[]) {
            Ok(plan) => match plan.spawn() {
                Ok(child) => {
                    self.child_pid = Some(child.id());
                    self.status = format!("Running {} (pid {})", profile.name, child.id());
                    self.child = Some(child);
                    if self.store.settings.close_on_launch {
                        self.max_frames = Some(self.frames + 1);
                    }
                }
                Err(error) => self.status = error.to_string(),
            },
            Err(error) => self.status = error.to_string(),
        }
    }

    fn poll_child(&mut self) {
        let Some(child) = &mut self.child else {
            return;
        };
        match child.try_wait() {
            Ok(Some(status)) => {
                self.status = format!(
                    "Game exited with {}",
                    status
                        .code()
                        .map(|code| code.to_string())
                        .unwrap_or_else(|| "signal".into())
                );
                self.child = None;
                self.child_pid = None;
                self.refresh_profile_views();
            }
            Ok(None) => {}
            Err(error) => {
                self.status = format!("Could not poll the game process: {error}");
                self.child = None;
                self.child_pid = None;
            }
        }
    }

    fn create_profile(&mut self) {
        let selection = platform::default_selection();
        match self
            .store
            .add_profile("New Profile", "1.21.11", &selection)
        {
            Ok(id) => {
                if let Err(error) = self.store.save() {
                    self.status = error.to_string();
                    return;
                }
                self.select_profile(&id);
                self.status = "Profile created".into();
            }
            Err(error) => self.status = error.to_string(),
        }
    }

    fn duplicate_profile(&mut self) {
        let Some(id) = self.selected.clone() else {
            return;
        };
        match self.store.duplicate_profile(&id) {
            Ok(new_id) => {
                let _ = self.store.save();
                self.select_profile(&new_id);
                self.status = "Profile duplicated".into();
            }
            Err(error) => self.status = error.to_string(),
        }
    }

    fn remove_profile(&mut self) {
        let Some(id) = self.selected.clone() else {
            return;
        };
        match self.store.remove_profile(&id) {
            Ok(()) => {
                let _ = self.store.save();
                let next = self.store.active.clone();
                if let Some(next) = next {
                    self.select_profile(&next);
                }
                self.status = "Profile removed".into();
            }
            Err(error) => self.status = error.to_string(),
        }
    }

    fn add_account(&mut self) {
        let name = self.account_name.trim();
        if name.is_empty() {
            self.status = "Enter a player name first".into();
            return;
        }
        match Account::offline(name) {
            Ok(account) => match self.store.add_account(account) {
                Ok(id) => {
                    let _ = self.store.set_active_account(&id);
                    let _ = self.store.save();
                    self.account_name.clear();
                    self.status = "Offline account added".into();
                }
                Err(error) => self.status = error.to_string(),
            },
            Err(error) => self.status = error.to_string(),
        }
    }

    fn check_updates(&mut self) {
        if self.update_url_text.trim().is_empty() {
            self.update_status = "Set an update manifest URL first".into();
            return;
        }
        match launcher_core::update::fetch_manifest(self.update_url_text.trim())
            .and_then(|bytes| UpdateManifest::from_json(&bytes))
        {
            Ok(manifest) => match manifest.verify_release() {
                Ok(()) => {
                    let platform = platform::Platform::current().name();
                    let arch = platform::arch();
                    let artifacts = manifest.artifacts_for(platform, arch).len();
                    self.update_status = format!(
                        "{} ({}) signed; {} artifact(s) for {} {}",
                        manifest.version, manifest.channel, artifacts, platform, arch
                    );
                    self.update_manifest = Some(manifest);
                }
                Err(error) => {
                    self.update_status = format!("Signature rejected: {error}");
                    self.update_manifest = None;
                }
            },
            Err(error) => {
                self.update_status = format!("Update check failed: {error}");
                self.update_manifest = None;
            }
        }
    }

    fn upload_crash(&mut self) {
        let Some(report) = self.selected_crash.clone() else {
            return;
        };
        let Some(endpoint) = non_empty(&self.crash_url_text) else {
            self.status = "Set the crash upload endpoint first".into();
            return;
        };
        #[cfg(feature = "upload")]
        {
            match logs::upload_crash_report(
                &report,
                endpoint,
                self.store.settings.upload_crash_reports,
            ) {
                Ok(logs::UploadOutcome::Disabled) => {
                    self.status = "Crash upload is disabled (opt-in unchecked)".into()
                }
                Ok(logs::UploadOutcome::Uploaded(status)) => {
                    self.status = format!("Crash report uploaded (HTTP {status})")
                }
                Err(error) => self.status = error.to_string(),
            }
        }
        #[cfg(not(feature = "upload"))]
        {
            let _ = endpoint;
            self.status = "This build was compiled without upload support".into();
        }
    }

    fn header(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::top("header")
            .frame(egui::Frame::NONE.fill(BG).inner_margin(egui::Margin::symmetric(18, 12)))
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.vertical(|ui| {
                        ui.label(
                            RichText::new("MATTICRAFT")
                                .size(24.0)
                                .strong()
                                .color(ACCENT),
                        );
                        ui.label(
                            RichText::new(format!(
                                "Launcher {}  ·  {} {}  ·  default {}",
                                env!("CARGO_PKG_VERSION"),
                                platform::Platform::current().name(),
                                platform::arch(),
                                platform::default_selection()
                            ))
                            .size(11.0)
                            .color(MUTED),
                        );
                    });
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        for (tab, label) in [
                            (Tab::Settings, "Settings"),
                            (Tab::Logs, "Logs"),
                            (Tab::Mods, "Mods"),
                            (Tab::Profiles, "Profiles"),
                        ] {
                            if ui
                                .selectable_label(self.tab == tab, RichText::new(label).strong())
                                .clicked()
                            {
                                self.tab = tab;
                            }
                        }
                    });
                });
            });
    }

    fn sidebar(&mut self, ctx: &egui::Context) {
        egui::SidePanel::left("profiles")
            .resizable(false)
            .default_width(286.0)
            .frame(egui::Frame::NONE.fill(PANEL).inner_margin(egui::Margin::same(14)))
            .show(ctx, |ui| {
                ui.label(RichText::new("PROFILES").size(11.0).color(MUTED));
                ui.add_space(6.0);

                let mut select = None;
                egui::ScrollArea::vertical()
                    .id_salt("profile-list")
                    .max_height(220.0)
                    .auto_shrink([false, false])
                    .show(ui, |ui| {
                        for profile in &self.store.profiles {
                            let active = self.selected.as_deref() == Some(profile.id.as_str());
                            let label = format!(
                                "{}  ·  {}",
                                profile.name,
                                profile.display_selection()
                            );
                            if ui
                                .add_sized(
                                    [ui.available_width(), 34.0],
                                    egui::SelectableLabel::new(active, label),
                                )
                                .clicked()
                            {
                                select = Some(profile.id.clone());
                            }
                        }
                    });
                if let Some(id) = select {
                    self.select_profile(&id);
                }

                ui.add_space(6.0);
                ui.horizontal(|ui| {
                    if ui.button("New").clicked() {
                        self.create_profile();
                    }
                    if ui.button("Copy").clicked() {
                        self.duplicate_profile();
                    }
                    if ui
                        .add_enabled(self.store.profiles.len() > 1, egui::Button::new("Remove"))
                        .clicked()
                    {
                        self.remove_profile();
                    }
                });

                ui.add_space(14.0);
                ui.separator();
                ui.add_space(8.0);
                ui.label(RichText::new("ACCOUNT").size(11.0).color(MUTED));
                let mut activate = None;
                let mut remove = None;
                for account in &self.store.accounts {
                    let active = self.store.active_account.as_deref() == Some(account.id.as_str());
                    ui.horizontal(|ui| {
                        if ui
                            .selectable_label(
                                active,
                                format!("{}  ({:?})", account.username, account.kind),
                            )
                            .clicked()
                        {
                            activate = Some(account.id.clone());
                        }
                        if ui.small_button("×").clicked() {
                            remove = Some(account.id.clone());
                        }
                    });
                }
                if let Some(id) = activate {
                    let _ = self.store.set_active_account(&id);
                    let _ = self.store.save();
                }
                if let Some(id) = remove {
                    let _ = self.store.remove_account(&id);
                    let _ = self.store.save();
                }
                ui.horizontal(|ui| {
                    ui.add(
                        egui::TextEdit::singleline(&mut self.account_name)
                            .hint_text("Player name")
                            .desired_width(150.0),
                    );
                    if ui.button("Add").clicked() {
                        self.add_account();
                    }
                });

                ui.add_space(18.0);
                let play = ui.add_sized(
                    [ui.available_width(), 46.0],
                    egui::Button::new(RichText::new("▶  PLAY").size(18.0).strong())
                        .fill(ACCENT_DARK),
                );
                if play.clicked() {
                    self.play();
                }
                if self.dirty {
                    ui.label(
                        RichText::new("Unsaved changes")
                            .size(11.0)
                            .color(Color32::from_rgb(240, 190, 90)),
                    );
                }
                ui.add_space(6.0);
                match find_game_binary(&self.store.settings) {
                    Ok(path) => {
                        ui.label(
                            RichText::new(format!("game: {}", path.display()))
                                .size(10.0)
                                .color(MUTED),
                        );
                    }
                    Err(_) => {
                        ui.label(
                            RichText::new("game binary not found - set it in Settings")
                                .size(10.0)
                                .color(Color32::from_rgb(240, 120, 100)),
                        );
                    }
                }
            });
    }

    fn profile_tab(&mut self, ui: &mut egui::Ui) {
        if let Some(error) = &self.open_error {
            ui.label(
                RichText::new(format!("Could not open the launcher store: {error}"))
                    .color(Color32::from_rgb(240, 120, 100)),
            );
            ui.add_space(8.0);
        }
        ui.label(RichText::new("PROFILE").size(12.0).color(MUTED));
        ui.add_space(4.0);
        let Some(draft) = &mut self.draft else {
            ui.label("No profile selected.");
            return;
        };

        let mut changed = false;
        let mut save = false;
        let mut reload = false;
        let mut open_instance = false;

        egui::Grid::new("profile-grid")
            .num_columns(2)
            .spacing([18.0, 10.0])
            .show(ui, |ui| {
                ui.label("Name");
                changed |= ui
                    .add(egui::TextEdit::singleline(&mut draft.name).desired_width(320.0))
                    .changed();
                ui.end_row();

                ui.label("Game version");
                changed |= ui
                    .add(egui::TextEdit::singleline(&mut draft.game_version).desired_width(180.0))
                    .changed();
                ui.end_row();

                ui.label("Renderer");
                egui::ComboBox::from_id_salt("profile-selection")
                    .width(240.0)
                    .selected_text(draft.selection.clone())
                    .show_ui(ui, |ui| {
                        for preset in PRESETS {
                            if ui
                                .selectable_value(
                                    &mut draft.selection,
                                    (*preset).to_string(),
                                    *preset,
                                )
                                .changed()
                            {
                                changed = true;
                            }
                        }
                    });
                ui.end_row();

                ui.label("Window");
                ui.horizontal(|ui| {
                    changed |= ui
                        .add(egui::DragValue::new(&mut draft.width).range(320..=7680))
                        .changed();
                    ui.label("×");
                    changed |= ui
                        .add(egui::DragValue::new(&mut draft.height).range(240..=4320))
                        .changed();
                    changed |= ui.checkbox(&mut draft.fullscreen, "fullscreen").changed();
                });
                ui.end_row();

                ui.label("Memory");
                ui.horizontal(|ui| {
                    changed |= ui
                        .add(
                            egui::DragValue::new(&mut draft.memory_mb)
                                .range(512..=65536)
                                .suffix(" MiB"),
                        )
                        .changed();
                });
                ui.end_row();

                ui.label("Instance");
                ui.monospace(draft.instance_dir.display().to_string());
                ui.end_row();

                ui.label("Mods");
                ui.monospace(draft.mods_dir.display().to_string());
                ui.end_row();

                ui.label("Logs");
                ui.monospace(draft.logs_dir.display().to_string());
                ui.end_row();
            });

        if changed {
            self.dirty = true;
        }
        ui.add_space(10.0);
        ui.horizontal(|ui| {
            if ui.button("Save profile").clicked() {
                save = true;
            }
            if ui.button("Reload").clicked() {
                reload = true;
            }
            if ui.button("Open instance folder").clicked() {
                open_instance = true;
            }
        });
        if open_instance {
            if let Some(draft) = &self.draft {
                self.open_path(draft.instance_dir.clone());
            }
        }
        if save {
            if let Err(error) = self.save_draft() {
                self.status = format!("Profile is not valid: {error}");
            }
        }
        if reload {
            if let Some(id) = self.selected.clone() {
                self.select_profile(&id);
            }
        }

        ui.add_space(18.0);
        ui.separator();
        ui.add_space(10.0);
        ui.label(RichText::new("LAUNCH ARGUMENTS").size(11.0).color(MUTED));
        if let Some(profile) = &self.draft {
            if let Ok(plan) = LaunchPlan::prepare(
                profile,
                &self.store.settings,
                self.store.active_account(),
                &[],
            ) {
                ui.monospace(plan.display_command());
            } else {
                ui.label(
                    RichText::new("Set a game binary to preview the command.")
                        .size(11.0)
                        .color(MUTED),
                );
            }
        }
    }

    fn mods_tab(&mut self, ui: &mut egui::Ui) {
        ui.label(RichText::new("MODS").size(12.0).color(MUTED));
        let Some(profile) = self.draft.clone() else {
            ui.label("No profile selected.");
            return;
        };
        ui.horizontal(|ui| {
            ui.monospace(profile.mods_dir.display().to_string());
            if ui.button("Rescan").clicked() {
                self.refresh_profile_views();
            }
            if ui.button("Open folder").clicked() {
                self.open_path(profile.mods_dir.clone());
            }
        });

        let Some(validation) = self.mod_validation.clone() else {
            return;
        };
        ui.add_space(8.0);
        let color = if validation.ok() { ACCENT } else { Color32::from_rgb(240, 120, 100) };
        ui.label(RichText::new(validation.summary()).strong().color(color));

        ui.horizontal(|ui| {
            if ui.button("Write manifest for current files").clicked() {
                match mods::build_manifest(&profile.mods_dir)
                    .and_then(|manifest| mods::write_manifest(&profile.mods_dir, &manifest))
                {
                    Ok(()) => {
                        self.status = "mods.json written".into();
                        self.refresh_profile_views();
                    }
                    Err(error) => self.status = error.to_string(),
                }
            }
        });

        ui.add_space(8.0);
        egui::ScrollArea::vertical()
            .max_height(340.0)
            .show(ui, |ui| {
                egui::Grid::new("mods-grid")
                    .striped(true)
                    .num_columns(3)
                    .spacing([16.0, 6.0])
                    .show(ui, |ui| {
                        ui.label(RichText::new("file").strong());
                        ui.label(RichText::new("size").strong());
                        ui.label(RichText::new("sha256").strong());
                        ui.end_row();
                        for file in &validation.installed {
                            ui.monospace(&file.file);
                            ui.monospace(format!("{} B", file.size));
                            ui.monospace(&file.sha256[..file.sha256.len().min(16)]);
                            ui.end_row();
                        }
                    });
            });

        if !validation.missing.is_empty() || !validation.mismatched.is_empty() {
            ui.add_space(8.0);
            ui.label(RichText::new("PROBLEMS").size(11.0).color(Color32::from_rgb(240, 120, 100)));
            for missing in &validation.missing {
                ui.monospace(format!("missing: {missing}"));
            }
            for changed in &validation.mismatched {
                ui.monospace(format!("changed: {}", changed.file));
            }
        }
        if !validation.unlisted.is_empty() {
            ui.add_space(6.0);
            ui.label(
                RichText::new(format!(
                    "{} file(s) are not listed in mods.json (allowed, but not verified).",
                    validation.unlisted.len()
                ))
                .size(11.0)
                .color(MUTED),
            );
        }
    }

    fn logs_tab(&mut self, ui: &mut egui::Ui) {
        ui.label(RichText::new("LOGS & CRASH REPORTS").size(12.0).color(MUTED));
        ui.horizontal(|ui| {
            if ui.button("Refresh").clicked() {
                self.refresh_profile_views();
            }
            if ui.button("Open logs folder").clicked() {
                if let Some(profile) = &self.draft {
                    self.open_path(profile.logs_dir.clone());
                }
            }
            if ui.button("Open crash folder").clicked() {
                if let Some(profile) = &self.draft {
                    self.open_path(logs::crash_reports_dir(&profile.instance_dir));
                }
            }
        });

        ui.add_space(8.0);
        ui.columns(2, |columns| {
            columns[0].label(RichText::new("latest.log").strong());
            egui::ScrollArea::vertical()
                .id_salt("log-tail")
                .max_height(420.0)
                .auto_shrink([false, false])
                .show(&mut columns[0], |ui| {
                    if self.log_lines.is_empty() {
                        ui.label(RichText::new("(no log yet)").color(MUTED));
                    }
                    for line in &self.log_lines {
                        ui.label(RichText::new(line).monospace().size(10.0));
                    }
                });

            columns[1].label(RichText::new("crash reports").strong());
            let mut show = None;
            for report in &self.crash_reports {
                let name = report
                    .file_name()
                    .map(|name| name.to_string_lossy().to_string())
                    .unwrap_or_default();
                let active = self.selected_crash.as_ref() == Some(report);
                if columns[1].selectable_label(active, name).clicked() {
                    show = Some(report.clone());
                }
            }
            if let Some(path) = show {
                self.show_crash(path);
            }
            if self.crash_reports.is_empty() {
                columns[1].label(RichText::new("(none)").color(MUTED));
            }
            columns[1].add_space(8.0);
            let mut consent = self.store.settings.upload_crash_reports;
            if columns[1]
                .checkbox(&mut consent, "Allow crash report upload (opt-in)")
                .changed()
            {
                self.store.settings.upload_crash_reports = consent;
                let _ = self.store.save();
            }
            if columns[1]
                .add_enabled(
                    self.selected_crash.is_some() && consent,
                    egui::Button::new("Upload selected report"),
                )
                .clicked()
            {
                self.upload_crash();
            }
        });

        if !self.crash_text.is_empty() {
            ui.add_space(10.0);
            ui.label(RichText::new("REPORT").size(11.0).color(MUTED));
            egui::ScrollArea::vertical()
                .id_salt("crash-text")
                .max_height(220.0)
                .show(ui, |ui| {
                    ui.monospace(&self.crash_text);
                });
        }
    }

    fn settings_tab(&mut self, ui: &mut egui::Ui) {
        ui.label(RichText::new("SETTINGS").size(12.0).color(MUTED));
        ui.monospace(self.store.root().display().to_string());
        ui.add_space(10.0);

        egui::Grid::new("settings-grid")
            .num_columns(2)
            .spacing([18.0, 10.0])
            .show(ui, |ui| {
                ui.label("Game binary");
                ui.horizontal(|ui| {
                    ui.add(
                        egui::TextEdit::singleline(&mut self.game_binary_text)
                            .hint_text("auto-detect matticraft/runClient")
                            .desired_width(420.0),
                    );
                    if ui.button("Clear").clicked() {
                        self.game_binary_text.clear();
                    }
                });
                ui.end_row();

                ui.label("Update manifest");
                ui.add(
                    egui::TextEdit::singleline(&mut self.update_url_text)
                        .hint_text("https://…/update.json or a local path")
                        .desired_width(420.0),
                );
                ui.end_row();

                ui.label("Crash upload endpoint");
                ui.add(
                    egui::TextEdit::singleline(&mut self.crash_url_text)
                        .hint_text("https://…/crash (only with opt-in)")
                        .desired_width(420.0),
                );
                ui.end_row();

                ui.label("Channel");
                ui.text_edit_singleline(&mut self.store.settings.channel);
                ui.end_row();

                ui.label("After launch");
                ui.checkbox(&mut self.store.settings.close_on_launch, "close the launcher");
                ui.end_row();
            });

        ui.add_space(10.0);
        ui.horizontal(|ui| {
            if ui.button("Save settings").clicked() {
                if let Err(error) = self.save_settings() {
                    self.status = error.to_string();
                }
            }
            if ui.button("Check updates").clicked() {
                self.check_updates();
            }
            if ui.button("Open data folder").clicked() {
                self.open_path(self.store.root().to_path_buf());
            }
        });
        if !self.update_status.is_empty() {
            ui.add_space(8.0);
            ui.label(&self.update_status);
        }
        if let Some(manifest) = &self.update_manifest {
            ui.add_space(10.0);
            ui.label(
                RichText::new(format!(
                    "{} · {} · {}",
                    manifest.version, manifest.channel, manifest.released_at
                ))
                .strong(),
            );
            for artifact in manifest.artifacts_for(
                platform::Platform::current().name(),
                platform::arch(),
            ) {
                ui.monospace(format!(
                    "{}  {}  {}  {}",
                    artifact.kind, artifact.name, artifact.sha256, artifact.url
                ));
            }
        }

        ui.add_space(18.0);
        ui.separator();
        ui.add_space(10.0);
        ui.label(RichText::new("DETECTION").size(11.0).color(MUTED));
        ui.monospace(format!(
            "platform: {} {} ({} bit)",
            platform::Platform::current().name(),
            platform::arch(),
            platform::bits()
        ));
        ui.monospace(format!(
            "default selection: {} (from {})",
            platform::default_selection(),
            platform::default_selection_source()
        ));
        ui.monospace(format!(
            "profiles: {}   accounts: {}",
            self.store.profiles.len(),
            self.store.accounts.len()
        ));
        if let Some(profile) = &self.draft {
            let crash_dir = logs::crash_reports_dir(&profile.instance_dir);
            let last = logs::list_crash_reports(&crash_dir)
                .ok()
                .and_then(|reports| reports.first().cloned());
            ui.monospace(format!(
                "crash reports: {}{}",
                logs::list_crash_reports(&crash_dir)
                    .map(|reports| reports.len())
                    .unwrap_or(0),
                last.map(|path| format!(" (latest {})", path.display()))
                    .unwrap_or_default()
            ));
        }
    }

    fn open_path(&mut self, path: PathBuf) {
        let _ = std::fs::create_dir_all(&path);
        #[cfg(target_os = "windows")]
        let mut command = std::process::Command::new("explorer");
        #[cfg(target_os = "macos")]
        let mut command = std::process::Command::new("open");
        #[cfg(all(unix, not(target_os = "macos")))]
        let mut command = std::process::Command::new("xdg-open");
        match command.arg(&path).spawn() {
            Ok(_) => self.status = format!("Opened {}", path.display()),
            Err(error) => self.status = format!("Could not open {}: {error}", path.display()),
        }
    }

    fn status_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::bottom("status")
            .frame(egui::Frame::NONE.fill(BG).inner_margin(egui::Margin::symmetric(18, 8)))
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    if self.status.is_empty() {
                        ui.label(RichText::new("Ready").size(11.0).color(MUTED));
                    } else {
                        ui.label(RichText::new(&self.status).size(11.0));
                    }
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        if let Some(pid) = self.child_pid {
                            ui.label(RichText::new(format!("pid {pid}")).size(11.0).color(ACCENT));
                        }
                        if !self.store.settings.update_manifest_url.as_deref().unwrap_or("").is_empty()
                        {
                            ui.label(
                                RichText::new(format!(
                                    "updates: {}",
                                    self.store.settings.channel
                                ))
                                .size(11.0)
                                .color(MUTED),
                            );
                        }
                    });
                });
            });
    }
}

impl eframe::App for LauncherApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        self.poll_child();
        self.frames += 1;
        if let Some(max) = self.max_frames {
            if self.frames >= max {
                ctx.send_viewport_cmd(egui::ViewportCommand::Close);
            }
            ctx.request_repaint();
        }
        self.header(ctx);
        self.status_bar(ctx);
        self.sidebar(ctx);
        egui::CentralPanel::default()
            .frame(egui::Frame::NONE.fill(BG).inner_margin(egui::Margin::same(18)))
            .show(ctx, |ui| match self.tab {
                Tab::Profiles => self.profile_tab(ui),
                Tab::Mods => self.mods_tab(ui),
                Tab::Logs => self.logs_tab(ui),
                Tab::Settings => self.settings_tab(ui),
            });
    }
}

fn install_style(ctx: &egui::Context) {
    let mut visuals = egui::Visuals::dark();
    visuals.panel_fill = BG;
    visuals.window_fill = PANEL;
    visuals.extreme_bg_color = Color32::from_rgb(10, 12, 15);
    visuals.faint_bg_color = CARD;
    visuals.selection.bg_fill = ACCENT_DARK;
    visuals.selection.stroke.color = ACCENT;
    visuals.hyperlink_color = ACCENT;
    visuals.widgets.inactive.bg_fill = CARD;
    visuals.widgets.hovered.bg_fill = Color32::from_rgb(42, 48, 58);
    visuals.widgets.active.bg_fill = ACCENT_DARK;
    ctx.set_visuals(visuals);

    let mut style = (*ctx.style()).clone();
    style.spacing.item_spacing = egui::vec2(8.0, 8.0);
    style.spacing.button_padding = egui::vec2(10.0, 6.0);
    style.spacing.interact_size.y = 26.0;
    ctx.set_style(style);
}

fn non_empty(value: &str) -> Option<&str> {
    let value = value.trim();
    (!value.is_empty()).then_some(value)
}

fn non_empty_path(value: &str) -> Option<PathBuf> {
    non_empty(value).map(PathBuf::from)
}

#[allow(dead_code)]
fn _assert_path(_: &Path) {}
