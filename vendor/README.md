# Vendored material

This folder holds the **upstream reference sources** the port is built 1:1
against. They are *not* part of the build (with one exception, see below) and
keep their original licenses.

| Path | What |
|---|---|
| `bootstraplauncher/` | cpw's BootstrapLauncher (bsl reference) |
| `securejarhandler/` | cpw's SecureJarHandler (sjh reference) |
| `modlauncher/` | cpw's ModLauncher |
| `FancyModLoader/` | NeoForge FML + earlydisplay (incl. the Monocraft font) |
| `NeoForge/` | NeoForge itself (neoforgespi, registries, GameData, …) |
| `MCP-Reborn/` | Minecraft client sources (render/world reference) |
| `brigadier/`, `DataFixerUpper/`, `fastutil/`, `JOML/`, `netty/`, `night-config/`, `maven/`, `commons-lang/`, `Bus/`, `JarJar/`, `AccessTransformer/`, `sponge-mixin/` | library references |
| `stb/` | **tracked**: the stb_truetype the font port binds (like LWJGL does) |

## Why most of it is untracked

The clones are big (hundreds of MB) and only *reference* — the C port never
links them. They are re-fetched with:

```bash
tools/vendor-sync.sh
```

Everything the build actually reads at compile/run time is tracked explicitly
(see the negations in `.gitignore`).
