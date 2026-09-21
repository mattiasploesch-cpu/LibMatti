#!/usr/bin/env bash
# Re-fetches the untracked vendor reference clones. Everything the build reads
# is tracked in git; these clones are reference material only.
set -euo pipefail

cd "$(dirname "$0")/.."

repos=(
  "https://github.com/cpw/bootstraplauncher"
  "https://github.com/cpw/securejarhandler"
  "https://github.com/cpw/modlauncher"
  "https://github.com/neoforged/FancyModLoader"
  "https://github.com/neoforged/NeoForge"
  "https://github.com/brigadier/brigadier"
  "https://github.com/Mojang/DataFixerUpper"
  "https://github.com/vigna/fastutil"
  "https://github.com/JOML-CI/JOML"
  "https://github.com/netty/netty"
  "https://github.com/TheElectronWill/night-config"
  "https://github.com/apache/maven-artifact"
  "https://github.com/apache/commons-lang"
  "https://github.com/MinecraftForge/AccessTransformer"
)

for repo in "${repos[@]}"; do
  name="$(basename "$repo" .git)"
  if [ -d "vendor/$name" ]; then
    echo "vendor/$name already present, skipping"
  else
    echo "Cloning $repo ..."
    git clone --depth 1 "$repo" "vendor/$name"
  fi
done

echo "Done."
