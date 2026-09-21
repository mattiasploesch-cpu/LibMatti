# The import drop zone of the new_mod pipeline.
#
# A mod import PR adds exactly ONE .matti file here (label: `new_mod`). The
# mod-validate workflow checks the package, the mod-import workflow unpacks it
# into mods/custom/<mod_id>/ after the merge and places the built .so into
# mods/.

# Nothing in this folder is committed except this README: the .matti file of
# an open PR lives on the PR branch only.
*.matti
