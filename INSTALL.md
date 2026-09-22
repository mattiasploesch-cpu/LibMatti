# Matticraft installieren

Zwei Wege: **automatisch** über `tools/install.sh` (holt immer das neueste
Release) oder **manuell** aus den Release-Artefakten. Beide brauchen nur
Linux x86_64 mit OpenGL/GLFW-Treibern — kein Java, kein Python, keine
weiteren Laufzeiten.

---

## Automatisch (empfohlen)

```bash
curl -fsSL https://raw.githubusercontent.com/mattiasploesch-cpu/LibMatti/main/tools/install.sh -o install.sh
bash install.sh
```

Das Skript:

1. löst das neueste Release über die GitHub-API auf,
2. lädt die Portable-Zip (und das AppImage) herunter,
3. installiert nach `~/.local/share/matticraft` (kein Root nötig),
4. legt den Starter `~/.local/bin/matticraft` an.

Optionen:

| Befehl | Wirkung |
|--------|---------|
| `bash install.sh` | neuestes Release, User-Space (`~/.local`) |
| `bash install.sh v1.21.11-r1` | ein bestimmtes Release-Tag |
| `bash install.sh --root` | systemweit nach `/usr/local` (sudo, wenn nötig) |
| `bash install.sh --root v1.21.11-r1` | systemweit + festes Tag |
| `bash install.sh --help` | Hilfe |

Starten danach:

```bash
matticraft
```

Das Spiel-Verzeichnis liegt bei `~/.matticraft` (Configs, `mods/`, Logs).
Wer es woanders haben will: `MATTICRAFT_GAME_DIR=/pfad matticraft`.

Deinstallieren:

```bash
rm -rf ~/.local/share/matticraft ~/.local/bin/matticraft
# bzw. mit --root:
sudo rm -rf /usr/local/share/matticraft /usr/local/bin/matticraft
```

---

## Manuell

Alle Artefakte gibt es unter
**https://github.com/mattiasploesch-cpu/LibMatti/releases**.

### Variante 1: AppImage

Das AppImage ist die eine-Datei-Variante — Font, Manifests und Mods-Ordner
sind eingebettet bzw. liegen daneben, sobald das Spiel erstmals lief.

```bash
chmod +x Matticraft-<version>-x86_64.AppImage
./Matticraft-<version>-x86_64.AppImage
```

Optionen an das Spiel gehen hinter `--` durch bzw. direkt mit:

```bash
./Matticraft-<version>-x86_64.AppImage --appimage-extract   # entpacken
./squashfs-root/run-matticraft.sh                            # entpackt starten
```

Falls das System kein FUSE hat (`libfuse2` fehlt): entweder `libfuse2`
installieren (`sudo apt install libfuse2`) oder einfach Variante 2 nutzen.

### Variante 2: Portable-Zip (entpacken und starten)

```bash
unzip Matticraft-<version>-portable-linux-x64.zip
cd Matticraft-<version>/
./run-matticraft.sh
```

Der Starter setzt `--gameDir` auf den entpackten Ordner — alles (Configs,
`mods/`, Cache) bleibt in dem Ordner, nichts landet im Home.

Direkt ohne Starter:

```bash
./matticraft --launchTarget neoforge --gameDir .
```

### Variante 3: Mods ins Spiel bringen

In beiden Varianten gibt es einen `mods/`-Ordner neben der Binary
(AppImage: nach dem ersten Start im `--gameDir`):

```bash
cp examplemod-1.0.matti <gamedir>/mods/    # oder die nackte .so
```

### Variante 4: Dev-SDK (für Mod-Entwickler)

```bash
tar -xzf libmatti-dev-linux-x64.tar.gz -C ~/dev
# ~/dev/libmatti-dev/lib/libmatti.a + include/libmatti/... + libmatti-config.cmake
```

Das Client-Projekt (MDK) zeigt per `MATTI_SDK_DIR` auf den entpackten
Ordner und linkt gegen die statische Bibliothek.

---

## Font-Handling

Der FML-Theme-Font (Monocraft.ttf) ist **in die Binary eingebettet** —
kein Download, keine Pfade nötig. Reihenfolge beim Laden:

1. `MATTI_THEME_FONT=/pfad/Monocraft.ttf` (Override, z. B. für eigene Fonts),
2. der Repo-Pfad im Checkout (Dev-Builds),
3. die eingebettete Kopie (immer da).

---

## Voraussetzungen

| Paket | Wofür |
|-------|-------|
| OpenGL-Treiber (Mesa/Nvidia) | das Fenster + Rendering |
| `libglfw3` | Window/Input (in den meisten Distros vorinstalliert) |
| `libfuse2` | nur fürs direkte AppImage-Starten (sonst `--appimage-extract`) |

Arch/Fedora: `glfw` heißt dort gleich; die portable Zip braucht außer
OpenGL nichts Externes.

---

## Troubleshooting

**Kein Fenster auf Wayland** — das Fenster läuft über GLFW/X11:
`pkill Xwayland` vermeiden bzw. `xwayland` installieren.

**`ERROR: theme font not available`** — kommt in Release-Builds praktisch
nicht mehr vor (Font ist eingebettet). Falls doch: `MATTI_THEME_FONT` auf
eine Monocraft.ttf setzen.

**Zombie-Instanz** — das Spiel schreibt seinen PID-Lock in `config/`;
bei hartem Kill einmal `rm -f <gamedir>/config/*.lock`.

**Mods werden nicht geladen** — nur `.matti`/`.so` mit gültiger
`.matti_manifest`-ELF-Sektion werden erkannt; Log-Zeile
`[SCAN] DEBUG: Found ...` prüfen.
