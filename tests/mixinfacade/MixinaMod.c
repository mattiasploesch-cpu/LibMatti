// Fixture sources for the mixin facade harness. The harness needs two real mod
// files: mixina carries five [[mixins]] entries (one valid, one whose content is
// missing, one with a missing requiredMods, one asking for a newer behaviour
// version and one asking for an older one), mixinb references the same valid
// config to trigger the duplicate detection.
//
// The C files stay empty on purpose - the manifest and the neoforge.mods.toml
// come in through matti_add_mod/RESOURCES, the mixins.json rides on top via
// matti_add_section because its entry name carries no ELF-safe path.
