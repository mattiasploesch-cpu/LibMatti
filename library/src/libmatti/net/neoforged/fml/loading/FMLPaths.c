#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/java/nio/file/Path.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogUtils.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

static const char *const NAMES[LIBMATTI_FML_FMLPATHS_COUNT] = {
    "GAMEDIR", "JIJ_CACHEDIR", "MODSDIR", "CONFIGDIR", "FMLCONFIG"
};

// Java: private final Path relativePath; private final boolean isDirectory; private Path absolutePath;
static char *relativePaths[LIBMATTI_FML_FMLPATHS_COUNT];
static char *absolutePaths[LIBMATTI_FML_FMLPATHS_COUNT];

// Java: FMLCONFIG is the only non-directory constant
static int is_directory(LIBMATTI_FML_FMLPaths path)
{
    return path != LIBMATTI_FML_FMLPaths_FMLCONFIG;
}

const char *const *LIBMATTI_FML_FMLPaths_Names(size_t *count)
{
    *count = LIBMATTI_FML_FMLPATHS_COUNT;
    return NAMES;
}

// Java: relativePath = computePath(path) / parent.relativePath.resolve(computePath(path))
static void init_relative_paths(void)
{
    if (relativePaths[LIBMATTI_FML_FMLPaths_GAMEDIR] != NULL) return;

    relativePaths[LIBMATTI_FML_FMLPaths_GAMEDIR] = strdup("");
    relativePaths[LIBMATTI_FML_FMLPaths_JIJ_CACHEDIR] = strdup(".cache/jij");
    relativePaths[LIBMATTI_FML_FMLPaths_MODSDIR] = strdup("mods");
    relativePaths[LIBMATTI_FML_FMLPaths_CONFIGDIR] = strdup("config");
    relativePaths[LIBMATTI_FML_FMLPaths_FMLCONFIG] =
        LIBMATTI_JNF_Path_Resolve(relativePaths[LIBMATTI_FML_FMLPaths_CONFIGDIR], "fml.toml");
}

// Java: @ApiStatus.Internal public static void loadAbsolutePaths(Path rootPath)
void LIBMATTI_FML_FMLPaths_LoadAbsolutePaths(const char *rootPath)
{
    init_relative_paths();

    for (int i = 0; i < LIBMATTI_FML_FMLPATHS_COUNT; i++)
    {
        // Java: path.absolutePath = rootPath.resolve(path.relativePath).toAbsolutePath().normalize();
        char *resolved = LIBMATTI_JNF_Path_Resolve(rootPath, relativePaths[i]);
        free(absolutePaths[i]);
        absolutePaths[i] = LIBMATTI_JNF_Path_ToAbsolutePath(resolved);
        free(resolved);

        // Java: if (path.isDirectory && !Files.isDirectory(path.absolutePath)) { Files.createDirectories(...) }
        if (is_directory(i) && !LIBMATTI_JNF_Files_IsDirectory(absolutePaths[i]))
        {
            if (!LIBMATTI_JNF_Files_CreateDirectories(absolutePaths[i]))
            {
                // Java: throw new UncheckedIOException(ioe);
                LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Failed to create directory {}",
                                         absolutePaths[i]);
            }
        }

        // Java: if (LOGGER.isDebugEnabled(CORE)) { LOGGER.debug(CORE, "Path {} is {}", path, path.absolutePath); }
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Path {} is {}", NAMES[i], absolutePaths[i]);
    }
}

// Java: public static Path getOrCreateGameRelativePath(Path path)
char *LIBMATTI_FML_FMLPaths_GetOrCreateGameRelativePath(const char *path)
{
    // Java: Path gameFolderPath = FMLPaths.GAMEDIR.get().resolve(path);
    char *gameFolderPath = LIBMATTI_JNF_Path_Resolve(LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths_GAMEDIR), path);

    // Java: if (!Files.isDirectory(gameFolderPath)) { Files.createDirectories(gameFolderPath); }
    if (!LIBMATTI_JNF_Files_IsDirectory(gameFolderPath))
    {
        if (!LIBMATTI_JNF_Files_CreateDirectories(gameFolderPath))
        {
            // Java: throw new RuntimeException(e);
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_CORE, "Failed to create directory {}",
                                     gameFolderPath);
        }
    }

    return gameFolderPath;
}

// Java: public Path relative()
const char *LIBMATTI_FML_FMLPaths_Relative(LIBMATTI_FML_FMLPaths path)
{
    init_relative_paths();
    return relativePaths[path];
}

// Java: public Path get()
const char *LIBMATTI_FML_FMLPaths_Get(LIBMATTI_FML_FMLPaths path)
{
    return absolutePaths[path];
}
