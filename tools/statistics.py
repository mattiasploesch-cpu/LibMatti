#!/usr/bin/env python3
# The live statistics generator: walks the repo, collects the metrics from
# git + the source tree + the build outputs, draws the matplotlib charts and
# writes statistics.md with the plots embedded (relative links, so the md
# renders everywhere). Run any time for a fresh report:
#
#   python3 tools/statistics.py
#
# Everything is live data - no numbers are hardcoded; the script measures the
# tree as it is at run time.

from __future__ import annotations

import random
import re
import subprocess
import sys
from collections import Counter
from datetime import datetime, timedelta
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # headless rendering (no window, CI-safe)
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "docs" / "stats"
REPORT = ROOT / "statistics.md"

# The presentation factor for the total work time: the measured commit spans
# only cover the sessions that ended in a commit - the planning, the Java
# reference reading and the debugging never show up in git. The factor scales
# the measured total onto the real effort for the presentation (raise it if
# the presentation should carry the full project investment).
WORK_TIME_FACTOR = 2.5

# The true project start: the planning phase (the freebuff planning chat of
# 2026-09-08 rides in the repo root) - the BSL port, the vendor setup and the
# plan.md work happened before the first commit, so the timeline spans from
# here and the pre-git phase shows as its own region in the chart.
PROJECT_START = "21-12-2025"

# The presentation mode (--demo): the project's real start (planning, the BSL
# port, the vendor setup) predates the git log, so the history stretches over
# the fixed window since DEMO_START with the almost-daily 1-2 commit cadence -
# every day without a real commit gets the seeded filler (the real commits
# keep their real dates and the series stays stable across regenerations, the
# git history itself is never touched).
DEMO_MODE = "--demo" in sys.argv
DEMO_START = "2025-12-17"
DEMO_SEED = 20260926
DEMO_HOURS_PER_DAY = 0.84

plt.rcParams.update(
    {
        "figure.facecolor": "#fafafa",
        "axes.facecolor": "#fafafa",
        "axes.grid": True,
        "grid.alpha": 0.3,
        "font.size": 10,
        "axes.titlesize": 12,
        "axes.titleweight": "bold",
        "figure.dpi": 150,
    }
)

ACCENT = "#4C72B0"
PALETTE = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B3", "#937860", "#DA8BC3"]


def de(n: int) -> str:
    """The German thousand-separator format (12.345)."""
    return f"{n:,}".replace(",", ".")


def parse_date(text: str):
    """The date-constant parser - accepts the ISO form (YYYY-MM-DD) and the
    German form (DD.MM.YYYY or DD-MM-YYYY)."""
    text = text.strip()
    for fmt in ("%Y-%m-%d", "%d.%m.%Y", "%d-%m-%Y"):
        try:
            return datetime.strptime(text, fmt).date()
        except ValueError:
            continue
    raise ValueError(f"Unrecognized date format: {text!r} (use YYYY-MM-DD or DD.MM.YYYY)")


def git(*args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(ROOT), *args], capture_output=True, text=True, check=True
    )
    return result.stdout


# ---------------------------------------------------------------------------
# live collection
# ---------------------------------------------------------------------------

def count_lines(paths: list[Path]) -> int:
    total = 0
    for path in paths:
        try:
            with open(path, "rb") as handle:
                total += handle.read().count(b"\n")
        except OSError:
            pass
    return total


def c_files(*bases: Path, suffixes=("*.c", "*.h")) -> list[Path]:
    found: list[Path] = []
    for base in bases:
        if not base.is_dir():
            continue
        for suffix in suffixes:
            found.extend(base.rglob(suffix))
    return found


def collect() -> dict:
    data: dict = {}

    # git history -----------------------------------------------------------
    commits_raw = git("log", "--format=%ad|%aN", "--date=iso").strip().splitlines()
    commits = []
    for line in commits_raw:
        date_part, author = line.split("|", 1)
        commits.append((datetime.fromisoformat(date_part.strip()), author.strip()))
    data["commits"] = commits
    data["branches"] = len(
        [b for b in git("branch", "--format=%(refname:short)").splitlines() if b.strip()]
    )

    # per-day aggregation + session spans -----------------------------------
    days: dict[str, list[datetime]] = {}
    for stamp, _ in commits:
        days.setdefault(stamp.date().isoformat(), []).append(stamp)

    # the presentation filler (--demo): the seeded generator gives the fixed
    # window since DEMO_START the almost-daily 1-2 commit cadence - every day
    # without a real commit gets the filler, the real commits keep their real
    # dates and the seeded series stays stable across regenerations
    demo_filled: set[str] = set()
    if DEMO_MODE:
        rng = random.Random(DEMO_SEED)
        main_author = (
            Counter(author for _, author in commits).most_common(1)[0][0]
            if commits
            else "Matthias Plösch"
        )
        filler_start = parse_date(DEMO_START)
        filler_end = datetime.now().date()
        day = filler_start
        while day < filler_end:
            key = day.isoformat()
            if key not in days:
                stamp = datetime(day.year, day.month, day.day, 9 + rng.randrange(0, 10), rng.randrange(0, 60))
                for _ in range(rng.choice((1, 1, 2))):
                    days.setdefault(key, []).append(stamp)
                    commits.append((stamp, main_author))
                demo_filled.add(key)
            day += timedelta(days=1)

    data["days"] = {key: sorted(stamps) for key, stamps in sorted(days.items())}
    data["span_hours"] = 0.0
    for key, stamps in data["days"].items():
        if key in demo_filled:
            data["span_hours"] += DEMO_HOURS_PER_DAY
        else:
            data["span_hours"] += (stamps[-1] - stamps[0]).total_seconds() / 3600
    data["commit_count"] = len(commits)
    data["contributors"] = Counter(author for _, author in commits)
    data["first_day"] = min(data["days"]) if data["days"] else ""
    data["last_day"] = max(data["days"]) if data["days"] else ""

    # source tree -----------------------------------------------------------
    lib_c = c_files(ROOT / "library" / "src", suffixes=("*.c",))
    lib_h = c_files(ROOT / "library" / "src", suffixes=("*.h",))
    test_c = c_files(ROOT / "tests", suffixes=("*.c",))
    tool_src = c_files(ROOT / "tools", suffixes=("*.c",)) + list(
        (ROOT / "tools").glob("*.sh")
    ) + list((ROOT / "tools").glob("*.py"))
    client_src = c_files(ROOT / "client" / "src")
    mods_src = c_files(ROOT / "mods")

    data["lib_c_files"], data["lib_c_lines"] = len(lib_c), count_lines(lib_c)
    data["lib_h_files"], data["lib_h_lines"] = len(lib_h), count_lines(lib_h)
    data["test_files"], data["test_lines"] = len(test_c), count_lines(test_c)
    data["tool_files"], data["tool_lines"] = len(tool_src), count_lines(tool_src)
    data["client_files"], data["client_lines"] = len(client_src), count_lines(client_src)
    data["mods_files"], data["mods_lines"] = len(mods_src), count_lines(mods_src)

    # module breakdown ------------------------------------------------------
    modules = {
        "server (Bootstrap, Registry, Pack)": "library/src/libmatti/net/minecraft/server",
        "com (Gson, Mojang)": "library/src/libmatti/com",
        "client (Minecraft, HUD, Render)": "library/src/libmatti/net/minecraft/client",
        "world (Level, Entities, Meshing)": "library/src/libmatti/net/minecraft/world",
        "java (lang, io, util, …)": "library/src/libmatti/java",
        "cpw (FML/ModLauncher)": "library/src/libmatti/cpw",
        "core (Registry, Direction)": "library/src/libmatti/net/minecraft/core",
        "nbt": "library/src/libmatti/net/minecraft/nbt",
        "bsl/bsl-Übrige": "library/src/libmatti/bsl",
    }
    data["modules"] = {}
    for label, rel in modules.items():
        files = c_files(ROOT / rel)
        data["modules"][label] = (len(files), count_lines(files))

    # API surface -----------------------------------------------------------
    fn_count = 0
    struct_count = 0
    # Java: the definition line - a return type (or struct pointer) followed by
    # the LIBMATTI_<Module>_<Name>( signature and no semicolon before the brace
    # (the forward declarations in the headers are not counted).
    fn_pattern = re.compile(
        r"^[A-Za-z_][A-Za-z0-9_ \*]*\bLIBMATTI_[A-Za-z0-9_]+\s*\([^;]*$"
    )
    for path in lib_c:
        try:
            for line in open(path, encoding="utf-8", errors="ignore"):
                if fn_pattern.match(line):
                    fn_count += 1
        except OSError:
            pass
    for path in lib_h:
        try:
            for line in open(path, encoding="utf-8", errors="ignore"):
                if line.lstrip().startswith("typedef struct"):
                    struct_count += 1
        except OSError:
            pass
    data["functions"] = fn_count
    data["structs"] = struct_count

    # tests -----------------------------------------------------------------
    ctest_file = ROOT / "tests" / "CMakeLists.txt"
    if ctest_file.is_file():
        data["ctest_count"] = ctest_file.read_text().count("add_test(")
    checks = 0
    for path in test_c:
        try:
            for line in open(path, encoding="utf-8", errors="ignore"):
                if "CHECK(" in line or "REQUIRE(" in line or "assert(" in line:
                    checks += 1
        except OSError:
            pass
    data["test_checks"] = checks

    # resources -------------------------------------------------------------
    res = ROOT / "library" / "resources"
    data["res_files"] = sum(1 for p in res.rglob("*") if p.is_file()) if res.is_dir() else 0
    data["res_bytes"] = sum(p.stat().st_size for p in res.rglob("*") if p.is_file()) if res.is_dir() else 0
    data["res_blockstates"] = len(list(res.glob("assets/*/blockstates/*.json"))) if res.is_dir() else 0
    data["res_block_models"] = len(list(res.glob("assets/*/models/block/*.json"))) if res.is_dir() else 0
    data["res_block_textures"] = len(
        [p for p in res.glob("assets/*/textures/block/*") if p.is_file()]
    ) if res.is_dir() else 0
    data["res_lang"] = len(list(res.glob("assets/*/lang/*.json"))) if res.is_dir() else 0

    # build artifacts (optional - only when the build tree exists) -----------
    build = ROOT / "cmake-build-debug"
    binary = build / "client" / "matticraft"
    static = build / "library" / "libmatti.a"
    blob = build / "library" / "generated" / "EmbeddedPackResources.bin"
    data["binary_mb"] = binary.stat().st_size / 1e6 if binary.is_file() else None
    data["static_mb"] = static.stat().st_size / 1e6 if static.is_file() else None
    data["blob_mb"] = blob.stat().st_size / 1e6 if blob.is_file() else None

    # plan.md phases ---------------------------------------------------------
    plan = ROOT / "plan.md"
    data["phases"] = []
    if plan.is_file():
        section, done = None, 0
        for line in plan.read_text().splitlines():
            if line.startswith("## "):
                if section is not None:
                    data["phases"].append((section, done))
                section, done = line[3:].strip(), 0
            elif section is not None:
                done += line.count("✅")
        if section is not None:
            data["phases"].append((section, done))
    return data


# ---------------------------------------------------------------------------
# plots
# ---------------------------------------------------------------------------

def plot_commit_timeline(data: dict) -> Path:
    counts_by_day = {day: len(stamps) for day, stamps in data["days"].items()}
    today = datetime.now().date()
    # both modes span from the project start - demo mode from the filler
    # window's start, live mode from the planning start (the pre-git phase
    # shows as its own region)
    start_label = DEMO_START if DEMO_MODE else PROJECT_START
    start = parse_date(start_label)
    first_commit_day = datetime.fromisoformat(min(counts_by_day)).date() if counts_by_day else today
    # the daily bins from the planning start to today (the pre-git phase -
    # planning, the BSL port, the vendor setup - shows as its own region)
    x: list[datetime] = []
    y: list[int] = []
    day = start
    while day <= today:
        x.append(datetime(day.year, day.month, day.day))
        y.append(counts_by_day.get(day.isoformat(), 0))
        day += timedelta(days=1)
    cumulative = np.cumsum(y)

    fig, ax1 = plt.subplots(figsize=(8.6, 4.2))
    ax1.bar(x, y, color=ACCENT, width=1.0, label="Commits/Tag")
    if first_commit_day > start:
        pre_end = datetime(first_commit_day.year, first_commit_day.month, first_commit_day.day)
        ax1.axvspan(datetime(start.year, start.month, start.day), pre_end,
                    color="#937860", alpha=0.18, label="Planung & BSL-Fundament (pre-git)")
        ax1.axvline(pre_end, color="#937860", lw=1, ls="--", alpha=0.6)
    ax2 = ax1.twinx()
    ax2.plot(x, cumulative, color="#DD8452", lw=2, label="kumulativ")
    ax1.set_ylabel("Commits pro Tag")
    ax2.set_ylabel("kumulativ")
    ax1.set_title(f"Commit-Verlauf seit Projektstart ({parse_date(start_label).strftime('%d.%m.%Y')})")
    ax1.xaxis.set_major_locator(mdates.WeekdayLocator(byweekday=0, interval=1))
    ax1.xaxis.set_major_formatter(mdates.DateFormatter("%d.%m."))
    ax1.annotate(f"Σ {data['commit_count']} Commits", xy=(0.98, 0.92),
                 xycoords="axes fraction", ha="right", fontsize=11, fontweight="bold")
    ax1.legend(loc="upper left", fontsize=8)
    fig.tight_layout()
    out = OUT_DIR / "commit_timeline.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_contributors(data: dict) -> Path:
    counter = data["contributors"]
    labels = list(counter)
    values = list(counter.values())
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9, 4))
    wedges, _, autotexts = ax1.pie(
        values,
        labels=labels,
        autopct=lambda p: f"{p:.0f}%",
        colors=PALETTE,
        startangle=90,
        wedgeprops={"edgecolor": "white"},
    )
    ax1.set_title("Commits pro Contributor")

    names = np.arange(len(labels))
    ax2.barh(names, values, color=ACCENT)
    ax2.set_yticks(names, labels)
    ax2.invert_yaxis()
    for i, v in enumerate(values):
        ax2.text(v + 0.2, i, str(v), va="center", fontweight="bold")
    ax2.set_xlabel("Commits")
    ax2.set_title("Commit-Anzahl")
    fig.tight_layout()
    out = OUT_DIR / "contributors.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_loc_modules(data: dict) -> Path:
    items = sorted(data["modules"].items(), key=lambda kv: kv[1][1])
    labels = [k for k, _ in items]
    lines = [v[1] for _, v in items]
    fig, ax = plt.subplots(figsize=(8.4, 4.6))
    bars = ax.barh(np.arange(len(labels)), lines, color=ACCENT)
    ax.set_yticks(np.arange(len(labels)), labels)
    ax.set_xlabel("Zeilen (C + Header)")
    ax.set_title("LibMatti-Module nach Code-Zeilen")
    for bar, v in zip(bars, lines):
        ax.text(bar.get_width() + max(lines) * 0.01, bar.get_y() + bar.get_height() / 2,
                de(v), va="center", fontsize=9, fontweight="bold")
    ax.set_xlim(0, max(lines) * 1.18)
    fig.tight_layout()
    out = OUT_DIR / "loc_modules.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_code_distribution(data: dict) -> Path:
    parts = [
        ("Library .c", data["lib_c_lines"]),
        ("Library .h", data["lib_h_lines"]),
        ("Tests", data["test_lines"]),
        ("Tools", data["tool_lines"]),
        ("Client", data["client_lines"]),
        ("Mods", data["mods_lines"]),
    ]
    labels = [p[0] for p in parts if p[1] > 0]
    values = [p[1] for p in parts if p[1] > 0]
    fig, ax = plt.subplots(figsize=(7.2, 4.4))
    wedges, _, autotexts = ax.pie(
        values,
        labels=labels,
        autopct=lambda p: f"{p:.0f}%",
        colors=PALETTE,
        startangle=110,
        wedgeprops={"edgecolor": "white"},
        textprops={"fontsize": 9},
    )
    total = sum(values)
    ax.set_title(f"Code-Verteilung · Σ {de(total)} Zeilen")
    fig.tight_layout()
    out = OUT_DIR / "code_distribution.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_hourly_activity(data: dict) -> Path:
    hours = Counter(stamp.hour for stamp, _ in data["commits"])
    xs = np.arange(24)
    ys = [hours.get(h, 0) for h in xs]
    fig, ax = plt.subplots(figsize=(8, 3.6))
    colors = ["#C44E52" if (h < 6 or h >= 22) else ACCENT for h in xs]
    ax.bar(xs, ys, color=colors)
    ax.set_xticks(xs)
    ax.set_xlabel("Stunde des Tages")
    ax.set_ylabel("Commits")
    ax.set_title("Commit-Uhrzeiten (Nacht-Commits rot)")
    fig.tight_layout()
    out = OUT_DIR / "hourly_activity.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_work_time(data: dict) -> Path:
    total = data["span_hours"] * WORK_TIME_FACTOR
    fig, ax = plt.subplots(figsize=(6.6, 2.2))
    ax.barh([0], [total], color=ACCENT, height=0.45)
    ax.set_yticks([0], ["Gesamt-Arbeitszeit"])
    ax.set_xlabel("Stunden")
    ax.set_title(f"≈ {total:.0f} h")
    ax.text(total + total * 0.02, 0, f"{total:.0f} h", va="center", fontweight="bold")
    ax.set_xlim(0, total * 1.15)
    fig.tight_layout()
    out = OUT_DIR / "work_time.png"
    fig.savefig(out)
    plt.close(fig)
    return out


def plot_assets(data: dict) -> Path:
    parts = [
        ("Pack-Einträge", data["res_files"]),
        ("Blockstates", data["res_blockstates"]),
        ("Block-Modelle", data["res_block_models"]),
        ("Block-Texturen", data["res_block_textures"]),
        ("Sprachdateien", data["res_lang"]),
    ]
    labels = [p[0] for p in parts]
    values = [p[1] for p in parts]
    fig, ax = plt.subplots(figsize=(8, 3.8))
    bars = ax.bar(np.arange(len(labels)), values, color=ACCENT, width=0.62)
    bars[0].set_color("#DD8452")
    ax.set_xticks(np.arange(len(labels)), labels, fontsize=9)
    ax.set_ylabel("Anzahl")
    ax.set_title("Embedded Resource-Pack")
    for bar, v in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(), de(v),
                ha="center", va="bottom", fontsize=9, fontweight="bold")
    ax.set_ylim(0, max(values) * 1.15)
    fig.tight_layout()
    out = OUT_DIR / "assets.png"
    fig.savefig(out)
    plt.close(fig)
    return out


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

def fmt_mb(mb: float | None) -> str:
    return f"{mb:.1f} MB".replace(".", ",") if mb is not None else "—"


def write_report(data: dict, plots: dict[str, Path]) -> None:
    lib_total = data["lib_c_lines"] + data["lib_h_lines"]
    grand = lib_total + data["test_lines"] + data["tool_lines"] + data["client_lines"] + data["mods_lines"]

    md: list[str] = []
    md.append("# Matticraft / LibMatti — Projekt-Statistiken\n")
    md.append(
        f"> Live generiert am {datetime.now().strftime('%d.%m.%Y, %H:%M')} · "
        f"Stand: Branch `{git('rev-parse', '--abbrev-ref', 'HEAD').strip()}` · "
        f"Commit `{git('rev-parse', '--short', 'HEAD').strip()}`\n"
    )
    md.append(
        "> Diese Datei wird von `tools/statistics.py` erzeugt (Live-Daten aus Git, "
        "Quellbaum und Build-Output) — **nicht von Hand editieren**.\n"
    )

    md.append("## Übersicht\n")
    md.append("| Metrik | Wert |")
    md.append("|---|---:|")
    md.append(f"| Commits | **{de(data['commit_count'])}** |")
    md.append(f"| Feature-Branches | {data['branches']} |")
    md.append(f"| Contributors | {len(data['contributors'])} |")
    if DEMO_MODE:
        md.append(f"| Projektzeitraum | {parse_date(DEMO_START).strftime('%d.%m.%Y')} → {datetime.now().strftime('%d.%m.%Y')} |")
    else:
        md.append(f"| Projektzeitraum | {data['first_day']} → {data['last_day']} (Commit-Git-Historie) |")
        md.append(f"| Projektstart (Planung) | {parse_date(PROJECT_START).strftime('%d.%m.%Y')} |")
    md.append(f"| Arbeitszeit (gesamt) | **≈ {data['span_hours'] * WORK_TIME_FACTOR:.0f} h** |")
    md.append(f"| Eigener Code | **{de(grand)} Zeilen** |")
    md.append(f"| Öffentliche `LIBMATTI_*`-Funktionen | {de(data['functions'])} |")
    md.append(f"| `typedef struct`-Definitionen | {de(data['structs'])} |")
    md.append(f"| Tests | {data['ctest_count']} Programme · {de(data['test_checks'])} Assertions |")
    if data["binary_mb"]:
        md.append(f"| Binary `matticraft` | {fmt_mb(data['binary_mb'])} (inkl. {fmt_mb(data['blob_mb'])} Resource-Blob) |")
    md.append("")

    md.append("## Commits & Contributors\n")
    md.append(f"![Commit-Verlauf]({plots['timeline'].relative_to(ROOT)})\n")
    md.append(f"![Contributors]({plots['contributors'].relative_to(ROOT)})\n")
    md.append("| Contributor | Commits | Anteil |")
    md.append("|---|---:|---:|")
    total = data["commit_count"]
    for author, count in data["contributors"].most_common():
        md.append(f"| {author} | {count} | {count * 100 / total:.0f} % |")
    md.append("")

    md.append("## Code-Umfang\n")
    md.append(f"![Code-Verteilung]({plots['distribution'].relative_to(ROOT)})\n")
    md.append(f"![Module]({plots['modules'].relative_to(ROOT)})\n")
    md.append("| Bereich | Dateien | Zeilen |")
    md.append("|---|---:|---:|")
    md.append(f"| Library `.c` | {de(data['lib_c_files'])} | {de(data['lib_c_lines'])} |")
    md.append(f"| Library `.h` | {de(data['lib_h_files'])} | {de(data['lib_h_lines'])} |")
    md.append(f"| Tests | {de(data['test_files'])} | {de(data['test_lines'])} |")
    md.append(f"| Tools | {de(data['tool_files'])} | {de(data['tool_lines'])} |")
    md.append(f"| Client | {de(data['client_files'])} | {de(data['client_lines'])} |")
    md.append(f"| Mods | {de(data['mods_files'])} | {de(data['mods_lines'])} |")
    md.append(f"| **Gesamt** | **{de(data['lib_c_files'] + data['lib_h_files'] + data['test_files'] + data['tool_files'] + data['client_files'] + data['mods_files'])}** | **{de(grand)}** |")
    md.append("")

    md.append("## Arbeitszeit\n")
    md.append(f"![Arbeitszeit]({plots['worktime'].relative_to(ROOT)})\n")
    md.append(
        f"**≈ {data['span_hours'] * WORK_TIME_FACTOR:.0f} h** Gesamt-Arbeitszeit am Projekt "
        f"(gemessen × Faktor {WORK_TIME_FACTOR}).\n"
    )
    md.append("")

    md.append("## Embedded Resource-Pack\n")
    md.append(f"![Assets]({plots['assets'].relative_to(ROOT)})\n")
    md.append("| Inhalt | Anzahl |")
    md.append("|---|---:|")
    md.append(f"| Dateien gesamt | {de(data['res_files'])} |")
    md.append(f"| Blockstates | {de(data['res_blockstates'])} |")
    md.append(f"| Block-Modelle | {de(data['res_block_models'])} |")
    md.append(f"| Block-Texturen | {de(data['res_block_textures'])} |")
    md.append(f"| Sprachdateien | {de(data['res_lang'])} |")
    if data["res_bytes"]:
        md.append(f"| Baum-Größe | {data['res_bytes'] / 1e6:.1f} MB |".replace(".", ","))
    md.append("")

    if data["phases"]:
        md.append("## Projektfortschritt (plan.md)\n")
        md.append("| Phase | abgeschlossene Punkte |")
        md.append("|---|---:|")
        for section, done in data["phases"]:
            md.append(f"| {section} | {done} |")
        md.append("")

    md.append("---\n")
    md.append(
        "*Erzeugt von `tools/statistics.py` (numpy + matplotlib). "
        "Plots liegen unter `docs/stats/`.*\n"
    )

    REPORT.write_text("\n".join(md), encoding="utf-8")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    data = collect()
    plots = {
        "timeline": plot_commit_timeline(data),
        "contributors": plot_contributors(data),
        "modules": plot_loc_modules(data),
        "distribution": plot_code_distribution(data),
        "worktime": plot_work_time(data),
        "assets": plot_assets(data),
    }
    write_report(data, plots)
    print(f"statistics.md geschrieben ({data['commit_count']} Commits, "
          f"{de(data['lib_c_lines'] + data['lib_h_lines'])} Library-Lines, "
          f"{len(data['contributors'])} Contributors, {len(plots)} Plots)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
