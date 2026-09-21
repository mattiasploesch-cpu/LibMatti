<!-- new_mod PR - this template is mandatory for mod import PRs (label: new_mod).
     CodeRabbit will review this PR automatically; every field below is checked
     by the mod-validate workflow. -->

## Mod package

- **Mod id:** `<!-- e.g. coolmod -->`
- **Mod version:** `<!-- e.g. 1.0 -->`
- **Package file:** `mods/import/<name>.matti`  <!-- exactly one file -->

## Checklist (required)

- [ ] Exactly **one** `.matti` file is added under `mods/import/`
- [ ] The mod id matches `^[a-z][a-z0-9_-]{1,63}$`
- [ ] The manifest (`Automatic-Module-Name`) matches the mod id above
- [ ] The mod source is embedded in the package (MDK pack does this) so the
      import can restore `mods/custom/<mod_id>/`
- [ ] The entry point `<modid>_init` exists and takes the
      `LIBMATTI_FML_FMLModContainer_ConstructorArgs *` argument
- [ ] The mod does **not** link against libmatti (headers only - the library
      is provided by the running game process)
- [ ] `neoforge.mods.toml` (if present) uses the same mod id and version
- [ ] The client boot with the mod did not crash locally (`runClient`)

## What does the mod do?

<!-- Short description: mixins used, blocks/items registered, configs, ... -->

## Validation

<!-- The mod-validate workflow reports here:
     - manifest parse
     - rebuild from embedded source
     - client boot with the mod loaded
     - CodeRabbit review of the embedded source -->

@coderabbitai please review the embedded mod source for this import.
