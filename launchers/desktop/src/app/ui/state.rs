//! UI-only state.
//!
//! The core store remains the source of truth for profiles, accounts and
//! settings. This module holds transient launcher state: which panel is open,
//! active filters, queued downloads, toasts and dialogs.

use std::collections::VecDeque;

/// Sections of the settings page.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SettingsSection {
    Appearance,
    Game,
    Java,
    Network,
    Privacy,
    About,
}

impl SettingsSection {
    pub const ALL: [SettingsSection; 6] = [
        SettingsSection::Appearance,
        SettingsSection::Game,
        SettingsSection::Java,
        SettingsSection::Network,
        SettingsSection::Privacy,
        SettingsSection::About,
    ];

    pub fn label(self) -> &'static str {
        match self {
            SettingsSection::Appearance => "Appearance",
            SettingsSection::Game => "Game",
            SettingsSection::Java => "Java",
            SettingsSection::Network => "Network",
            SettingsSection::Privacy => "Privacy",
            SettingsSection::About => "About",
        }
    }

    pub fn description(self) -> &'static str {
        match self {
            SettingsSection::Appearance => "Theme, density, motion and background.",
            SettingsSection::Game => "Game binary, channel and launch behaviour.",
            SettingsSection::Java => "Memory, renderer and per-profile defaults.",
            SettingsSection::Network => "Update manifest, mirrors and timeouts.",
            SettingsSection::Privacy => "Crash reports and local diagnostics.",
            SettingsSection::About => "Build information, credits and licenses.",
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum LibraryFilter {
    All,
    Installed,
    Available,
}

impl LibraryFilter {
    pub const ALL: [LibraryFilter; 3] = [
        LibraryFilter::All,
        LibraryFilter::Installed,
        LibraryFilter::Available,
    ];

    pub fn label(self) -> &'static str {
        match self {
            LibraryFilter::All => "All builds",
            LibraryFilter::Installed => "Installed",
            LibraryFilter::Available => "Available",
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ProfileLayout {
    Grid,
    List,
}

impl ProfileLayout {
    pub fn label(self) -> &'static str {
        match self {
            ProfileLayout::Grid => "Grid",
            ProfileLayout::List => "List",
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum LogLevel {
    All,
    Info,
    Warn,
    Error,
}

impl LogLevel {
    pub const ALL: [LogLevel; 4] = [
        LogLevel::All,
        LogLevel::Info,
        LogLevel::Warn,
        LogLevel::Error,
    ];

    pub fn label(self) -> &'static str {
        match self {
            LogLevel::All => "All",
            LogLevel::Info => "INFO",
            LogLevel::Warn => "WARN",
            LogLevel::Error => "ERROR",
        }
    }

    pub fn matches(self, line: &str) -> bool {
        match self {
            LogLevel::All => true,
            LogLevel::Info => !line.contains("WARN") && !line.contains("ERROR"),
            LogLevel::Warn => line.contains("WARN"),
            LogLevel::Error => line.contains("ERROR"),
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum CosmeticCategory {
    All,
    Hats,
    Capes,
    Wings,
    Emotes,
    Auras,
}

impl CosmeticCategory {
    pub const ALL: [CosmeticCategory; 6] = [
        CosmeticCategory::All,
        CosmeticCategory::Hats,
        CosmeticCategory::Capes,
        CosmeticCategory::Wings,
        CosmeticCategory::Emotes,
        CosmeticCategory::Auras,
    ];

    pub fn label(self) -> &'static str {
        match self {
            CosmeticCategory::All => "All",
            CosmeticCategory::Hats => "Hats",
            CosmeticCategory::Capes => "Capes",
            CosmeticCategory::Wings => "Wings",
            CosmeticCategory::Emotes => "Emotes",
            CosmeticCategory::Auras => "Auras",
        }
    }

    pub fn matches(self, category: &str) -> bool {
        match self {
            CosmeticCategory::All => true,
            CosmeticCategory::Hats => category.eq_ignore_ascii_case("hat"),
            CosmeticCategory::Capes => category.eq_ignore_ascii_case("cape"),
            CosmeticCategory::Wings => category.eq_ignore_ascii_case("wings"),
            CosmeticCategory::Emotes => category.eq_ignore_ascii_case("emote"),
            CosmeticCategory::Auras => category.eq_ignore_ascii_case("aura"),
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum NewsCategory {
    All,
    Release,
    Community,
    Devlog,
}

impl NewsCategory {
    pub const ALL: [NewsCategory; 4] = [
        NewsCategory::All,
        NewsCategory::Release,
        NewsCategory::Community,
        NewsCategory::Devlog,
    ];

    pub fn label(self) -> &'static str {
        match self {
            NewsCategory::All => "Everything",
            NewsCategory::Release => "Releases",
            NewsCategory::Community => "Community",
            NewsCategory::Devlog => "Devlog",
        }
    }

    pub fn matches(self, category: &str) -> bool {
        match self {
            NewsCategory::All => true,
            NewsCategory::Release => category.eq_ignore_ascii_case("release"),
            NewsCategory::Community => category.eq_ignore_ascii_case("community"),
            NewsCategory::Devlog => category.eq_ignore_ascii_case("devlog"),
        }
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ToastKind {
    Info,
    Success,
    Warning,
    Error,
}

impl ToastKind {
    pub fn label(self) -> &'static str {
        match self {
            ToastKind::Info => "INFO",
            ToastKind::Success => "DONE",
            ToastKind::Warning => "WARN",
            ToastKind::Error => "ERROR",
        }
    }
}

#[derive(Clone, Debug)]
pub struct Toast {
    pub id: u64,
    pub kind: ToastKind,
    pub title: String,
    pub message: String,
    pub created_at: f64,
}

impl Toast {
    pub fn is_expired(&self, now: f64) -> bool {
        now - self.created_at > 6.0
    }

    pub fn age(&self, now: f64) -> f32 {
        (now - self.created_at).max(0.0) as f32
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ConfirmAction {
    RemoveProfile(String),
    RemoveAccount(String),
    ResetPreferences,
    InstallGame(String),
    ApplyPatch(String),
}

#[derive(Clone, Debug)]
pub struct ConfirmDialog {
    pub title: String,
    pub message: String,
    pub confirm_label: String,
    pub action: ConfirmAction,
    pub destructive: bool,
}

#[derive(Clone, Debug)]
pub struct DownloadItem {
    pub version: String,
    pub asset: String,
    pub progress: f32,
    pub bytes: u64,
    pub total: u64,
    pub state: DownloadState,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum DownloadState {
    Queued,
    Running,
    Ready,
    Failed,
}

impl DownloadState {
    pub fn label(self) -> &'static str {
        match self {
            DownloadState::Queued => "QUEUED",
            DownloadState::Running => "DOWNLOADING",
            DownloadState::Ready => "READY",
            DownloadState::Failed => "FAILED",
        }
    }
}

#[derive(Clone, Debug)]
pub struct OnboardingState {
    pub open: bool,
    pub step: usize,
    pub completed: bool,
}

impl Default for OnboardingState {
    fn default() -> Self {
        OnboardingState {
            open: false,
            step: 0,
            completed: false,
        }
    }
}

impl OnboardingState {
    pub const STEPS: usize = 4;
}

/// Transient UI data that is not part of the launcher store.
#[derive(Clone, Debug)]
pub struct UiState {
    pub command_open: bool,
    pub command_query: String,
    pub command_selected: usize,
    pub notifications_open: bool,
    pub shortcuts_open: bool,
    pub confirm: Option<ConfirmDialog>,
    pub onboarding: OnboardingState,
    pub settings_section: SettingsSection,
    pub library_filter: LibraryFilter,
    pub profile_layout: ProfileLayout,
    pub downloads: Vec<DownloadItem>,
    pub mods_query: String,
    pub mods_issues_only: bool,
    pub logs_query: String,
    pub log_level: LogLevel,
    pub selected_crash: Option<usize>,
    pub server_query: String,
    pub selected_server: usize,
    pub server_dialog_open: bool,
    pub server_form_name: String,
    pub server_form_address: String,
    pub cosmetic_category: CosmeticCategory,
    pub cosmetic_query: String,
    pub selected_cosmetic: Option<usize>,
    pub news_category: NewsCategory,
    pub selected_news: usize,
    pub editor_open: bool,
    pub add_profile_open: bool,
    pub add_account_open: bool,
    pub add_profile_name: String,
    pub add_profile_version: String,
    pub add_profile_loader: String,
    pub show_background: bool,
    pub background_dim: f32,
    pub compact: bool,
    pub animations: bool,
    pub reduce_motion: bool,
    pub accent_index: usize,
    pub sidebar_collapsed: bool,
    pub show_release_notes: bool,
    pub page_enter: f32,
    pub hover_glow: f32,
    pub now: f64,
    pub last_tab_switch: f64,
    pub toasts: VecDeque<Toast>,
    next_toast: u64,
    max_toasts: usize,
}

impl Default for UiState {
    fn default() -> Self {
        UiState {
            command_open: false,
            command_query: String::new(),
            command_selected: 0,
            notifications_open: false,
            shortcuts_open: false,
            confirm: None,
            onboarding: OnboardingState::default(),
            settings_section: SettingsSection::Appearance,
            library_filter: LibraryFilter::All,
            profile_layout: ProfileLayout::Grid,
            downloads: Vec::new(),
            mods_query: String::new(),
            mods_issues_only: false,
            logs_query: String::new(),
            log_level: LogLevel::All,
            selected_crash: None,
            server_query: String::new(),
            selected_server: 0,
            server_dialog_open: false,
            server_form_name: String::new(),
            server_form_address: String::new(),
            cosmetic_category: CosmeticCategory::All,
            cosmetic_query: String::new(),
            selected_cosmetic: None,
            news_category: NewsCategory::All,
            selected_news: 0,
            editor_open: false,
            add_profile_open: false,
            add_account_open: false,
            add_profile_name: String::new(),
            add_profile_version: "1.21.11".into(),
            add_profile_loader: "native:auto".into(),
            show_background: true,
            background_dim: 0.62,
            compact: false,
            animations: true,
            reduce_motion: false,
            accent_index: 0,
            sidebar_collapsed: false,
            show_release_notes: false,
            page_enter: 1.0,
            hover_glow: 0.0,
            now: 0.0,
            last_tab_switch: 0.0,
            toasts: VecDeque::new(),
            next_toast: 1,
            max_toasts: 4,
        }
    }
}

impl UiState {
    pub fn tick(&mut self, ctx: &egui::Context) {
        self.now = ctx.input(|input| input.time);
        let speed = if self.animations && !self.reduce_motion {
            0.12
        } else {
            1.0
        };
        self.page_enter = (self.page_enter + speed).min(1.0);
        self.hover_glow = (self.hover_glow - 0.04).max(0.0);
        let now = self.now;
        self.toasts.retain(|toast| !toast.is_expired(now));
    }

    pub fn toast(&mut self, kind: ToastKind, title: impl Into<String>, message: impl Into<String>) {
        let toast = Toast {
            id: self.next_toast,
            kind,
            title: title.into(),
            message: message.into(),
            created_at: self.now,
        };
        self.next_toast += 1;
        self.toasts.push_back(toast);
        while self.toasts.len() > self.max_toasts {
            self.toasts.pop_front();
        }
    }

    pub fn success(&mut self, title: impl Into<String>, message: impl Into<String>) {
        self.toast(ToastKind::Success, title, message);
    }

    pub fn info(&mut self, title: impl Into<String>, message: impl Into<String>) {
        self.toast(ToastKind::Info, title, message);
    }

    pub fn warn(&mut self, title: impl Into<String>, message: impl Into<String>) {
        self.toast(ToastKind::Warning, title, message);
    }

    pub fn error(&mut self, title: impl Into<String>, message: impl Into<String>) {
        self.toast(ToastKind::Error, title, message);
    }

    pub fn ask(&mut self, title: impl Into<String>, message: impl Into<String>, action: ConfirmAction, destructive: bool) {
        self.confirm = Some(ConfirmDialog {
            title: title.into(),
            message: message.into(),
            confirm_label: if destructive { "Delete".into() } else { "Continue".into() },
            action,
            destructive,
        });
    }

    pub fn close_command(&mut self) {
        self.command_open = false;
        self.command_query.clear();
        self.command_selected = 0;
    }

    pub fn command_matches(&self, query: &str, label: &str, hint: &str) -> bool {
        let query = query.trim().to_ascii_lowercase();
        query.is_empty()
            || label.to_ascii_lowercase().contains(&query)
            || hint.to_ascii_lowercase().contains(&query)
    }

    pub fn download_for(&mut self, version: &str, asset: &str, total: u64) -> &mut DownloadItem {
        if let Some(index) = self
            .downloads
            .iter()
            .position(|item| item.version == version && item.asset == asset)
        {
            return &mut self.downloads[index];
        }
        self.downloads.insert(
            0,
            DownloadItem {
                version: version.to_string(),
                asset: asset.to_string(),
                progress: 0.0,
                bytes: 0,
                total,
                state: DownloadState::Queued,
            },
        );
        &mut self.downloads[0]
    }

    pub fn clear_finished_downloads(&mut self) {
        self.downloads
            .retain(|item| item.state != DownloadState::Ready);
    }

    pub fn reset_preferences(&mut self) {
        let show_background = self.show_background;
        let compact = self.compact;
        let animations = self.animations;
        let reduce_motion = self.reduce_motion;
        let accent_index = self.accent_index;
        self.show_background = show_background;
        self.compact = compact;
        self.animations = animations;
        self.reduce_motion = reduce_motion;
        self.accent_index = accent_index;
        self.background_dim = 0.62;
        self.profile_layout = ProfileLayout::Grid;
        self.cosmetic_category = CosmeticCategory::All;
        self.news_category = NewsCategory::All;
        self.log_level = LogLevel::All;
    }
}

// `egui` is used in the method signature above; importing it here keeps the
// module independent of the concrete launcher app.
use eframe::egui;
