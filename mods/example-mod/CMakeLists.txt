# Example mod. Build the whole mods project from the repository root with:
#
#     cmake -S mods -B mods/build
#     cmake --build mods/build
#
# The result is mods/examplemod-1.0.so, which FML finds when the game runs with
# --gameDir <repo root>.

matti_add_mod(examplemod
        MOD_ID examplemod
        MOD_VERSION 1.0
        SOURCES src/example_mod.c
        RESOURCES resources
)

# A file under resources/ is embedded under its own path, so a resource a mod
# normally ships is just a file:
#
#   resources/META-INF/neoforge.mods.toml  ->  "META-INF/neoforge.mods.toml"
#   resources/META-INF/services/<iface>    ->  "META-INF/services/<iface>"
#   resources/data/<modid>/...             ->  "data/<modid>/..."
#
#
# resources/META-INF/neoforge.mods.toml is the mod's metadata; FML reads it with the
# ported NightConfig TOML parser.
