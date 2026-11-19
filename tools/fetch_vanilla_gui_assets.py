#!/usr/bin/env python3
"""Fetch the vanilla GUI textures the C menus draw.

The client jars only carry 1x1 panorama stubs; the real faces live in the
launcher's asset index. The widget sheet, dirt background, logo and bitmap
font come straight out of the 1.20.1 jar.

    python3 tools/fetch_vanilla_gui_assets.py
"""
import json
import os
import sys
import urllib.request
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CACHE = os.path.join(ROOT, ".cache", "vanilla")
OUT = os.path.join(ROOT, "library", "src", "libmatti", "matti", "ui", "assets")
RESOURCES = "https://resources.download.minecraft.net"

# Textures inside the client jar.
JAR_SOURCES = {
    "1.20.1": [
        "gui/widgets.png",
        "gui/options_background.png",
        "gui/title/minecraft.png",
        "gui/title/edition.png",
        "font/ascii.png",
    ],
}

# Textures only in the asset index (the jar entries are 1x1 stubs).
INDEX_SOURCES = {
    "1.16.5": [
        "gui/title/background/panorama_0.png",
        "gui/title/background/panorama_1.png",
        "gui/title/background/panorama_2.png",
        "gui/title/background/panorama_3.png",
        "gui/title/background/panorama_4.png",
        "gui/title/background/panorama_5.png",
    ],
}


def fetch(url):
    request = urllib.request.Request(url, headers={"User-Agent": "matti-assets/1.0"})
    return urllib.request.urlopen(request, timeout=300)


def version_meta(manifest, version):
    entry = next(v for v in manifest["versions"] if v["id"] == version and v["type"] == "release")
    return json.load(fetch(entry["url"]))


def jar_for(meta, version):
    client = meta["downloads"]["client"]
    path = os.path.join(CACHE, f"client-{version}.jar")
    if not (os.path.exists(path) and os.path.getsize(path) == client["size"]):
        data = fetch(client["url"]).read()
        if len(data) != client["size"] or not data.startswith(b"PK"):
            sys.exit(f"bad client jar for {version}: {len(data)} bytes")
        os.makedirs(CACHE, exist_ok=True)
        with open(path, "wb") as f:
            f.write(data)
        print(f"saved {version} client jar ({len(data)} bytes)")
    return path


def write(name, data):
    target = os.path.join(OUT, name)
    os.makedirs(os.path.dirname(target), exist_ok=True)
    with open(target, "wb") as f:
        f.write(data)
    print(f"extracted {len(data):7d}  {name}")


def main():
    manifest = json.load(fetch("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json"))
    extracted = 0

    for version, wanted in JAR_SOURCES.items():
        meta = version_meta(manifest, version)
        with zipfile.ZipFile(jar_for(meta, version)) as zf:
            names = set(zf.namelist())
            for name in wanted:
                entry = "assets/minecraft/textures/" + name
                if entry not in names:
                    print(f"missing in {version}: {name}")
                    continue
                write(name, zf.read(entry))
                extracted += 1

    for version, wanted in INDEX_SOURCES.items():
        meta = version_meta(manifest, version)
        index = json.load(fetch(meta["assetIndex"]["url"]))
        for name in wanted:
            key = "minecraft/textures/" + name
            entry = index["objects"].get(key)
            if entry is None:
                print(f"missing in {version} index: {name}")
                continue
            data = fetch(f"{RESOURCES}/{entry['hash'][:2]}/{entry['hash']}").read()
            if len(data) != entry["size"]:
                print(f"size mismatch for {name}")
                continue
            write(name, data)
            extracted += 1

    print(f"{extracted} textures extracted to {OUT}")


if __name__ == "__main__":
    main()
