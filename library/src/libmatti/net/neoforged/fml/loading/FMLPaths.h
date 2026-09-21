// Port of net.neoforged.fml.loading.FMLPaths.

#ifndef MATTICRAFT_FML_FMLPATHS_H
#define MATTICRAFT_FML_FMLPATHS_H

#include <stddef.h>

// Java: public enum FMLPaths { GAMEDIR(), JIJ_CACHEDIR(".cache/jij"), MODSDIR("mods"), CONFIGDIR("config"),
//         FMLCONFIG(false, CONFIGDIR, "fml.toml") }
typedef enum
{
    LIBMATTI_FML_FMLPaths_GAMEDIR,
    LIBMATTI_FML_FMLPaths_JIJ_CACHEDIR,
    LIBMATTI_FML_FMLPaths_MODSDIR,
    LIBMATTI_FML_FMLPaths_CONFIGDIR,
    LIBMATTI_FML_FMLPaths_FMLCONFIG
} LIBMATTI_FML_FMLPaths;

#define LIBMATTI_FML_FMLPATHS_COUNT 5

// Java: static FMLPaths[] values()
const char *const *LIBMATTI_FML_FMLPaths_Names(size_t *count);

// Java: @ApiStatus.Internal public static void loadAbsolutePaths(Path rootPath)
void LIBMATTI_FML_FMLPaths_LoadAbsolutePaths(const char *rootPath);

// Java: public static Path getOrCreateGameRelativePath(Path path)
char *LIBMATTI_FML_FMLPaths_GetOrCreateGameRelativePath(const char *path);

// Java: public Path relative()
const char *LIBMATTI_FML_FMLPaths_Relative(LIBMATTI_FML_FMLPaths path);
// Java: public Path get()
const char *LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths path);

#endif //MATTICRAFT_FML_FMLPATHS_H
