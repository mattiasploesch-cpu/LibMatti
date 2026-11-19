// Native library loading with the search paths the port needs.
//
// Java: LWJGL ships its natives next to the launcher and resolves them through
// its own library path. The C port dlopen()s the system libraries instead. On
// NixOS those live in the Nix profiles, which the dynamic loader does not
// search: "dlopen(\"libglfw.so.3\")" fails even though glfw is installed in
// /run/current-system/sw/lib or ~/.nix-profile/lib.
//
// LIBMATTI_NATIVE_OpenLibrary tries the plain sonames first (so a normal
// distribution install keeps working), then the directories of
// MATTI_LIBRARY_PATH, NIX_PROFILES and the standard profile locations.

#ifndef MATTICRAFT_MATTI_NATIVE_LIBRARY_H
#define MATTICRAFT_MATTI_NATIVE_LIBRARY_H

#include <stddef.h>




void *LIBMATTI_NATIVE_OpenLibrary(const char *const *names);




#define MATTI_NATIVE_GLFW_SECTION ".matti_lib_glfw"
#define MATTI_NATIVE_GL_SECTION ".matti_lib_gl"





void *LIBMATTI_NATIVE_OpenEmbeddedLibrary(const char *section_name);




int LIBMATTI_NATIVE_ReadEmbeddedSection(const char *section_name, unsigned char **out_data,
                                        size_t *out_size);

#endif
