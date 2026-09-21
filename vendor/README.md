# Vendored reference sources

This folder holds the **upstream reference sources** every port file maps to
1:1. They are **git submodules** (see [.gitmodules](../.gitmodules)): the repo
tracks only the pinned revision, the clones arrive with the repository:

```bash
git clone --recurse-submodules https://github.com/mattiasploesch-cpu/LibMatti.git
# or, for an existing clone:
git submodule update --init --recursive
```

| Path | Reference for |
|---|---|
| `bootstraplauncher/` | cpw's BootstrapLauncher (bsl) |
| `securejarhandler/` | cpw's SecureJarHandler (sjh) |
| `modlauncher/` | cpw's ModLauncher |
| `FancyModLoader/` | NeoForge FML + earlydisplay (incl. the Monocraft font) |
| `NeoForge/` | NeoForge (neoforgespi, GameData, registries, …) |
| `MCP-Reborn/` | Minecraft client sources (render/world reference) |
| `sponge-mixin/` | Sponge Mixin |
| `brigadier/`, `DataFixerUpper/`, `fastutil/`, `JOML/`, `netty/`, `night-config/`, `maven/`, `commons-lang/`, `Bus/`, `JarJar/`, `AccessTransformer/` | library references |
| `stb/` | stb_truetype (the font port binds it like LWJGL does) |

The submodules keep their original licenses and are **not** built - the C port
never links them. Two exceptions the build reads at compile/run time:

- `vendor/stb/stb_truetype.h` (include path of the `matti` target)
- `vendor/FancyModLoader/.../theme/Monocraft.ttf` (read at runtime for the
  window title font)

## Updating a submodule

```bash
git -C vendor/<name> fetch && git -C vendor/<name> checkout <revision>
git add vendor/<name> && git commit -m "vendor: bump <name>"
```
