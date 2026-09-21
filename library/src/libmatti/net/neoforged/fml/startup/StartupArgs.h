#ifndef MATTICRAFT_FML_STARTUPARGS_H
#define MATTICRAFT_FML_STARTUPARGS_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"

#include <stddef.h>

// Java: public record StartupArgs(Path gameDirectory, boolean headless, @Nullable Dist dist, boolean cleanDist,
//         String[] programArgs, Set<File> claimedFiles, List<File> unclaimedClassPathEntries,
//         @Nullable ClassLoader parentClassLoader)
typedef struct
{
    char *gameDirectory;
    int headless;
    LIBMATTI_DIST_Dist dist;
    int cleanDist;
    int argc;
    char **programArgs;
    char **claimedFiles;
    size_t claimedFileCount;
    char **unclaimedClassPathEntries;
    size_t unclaimedClassPathEntryCount;
    // Java: @Nullable ClassLoader parentClassLoader (the ported java.lang.ClassLoader)
    void *parentClassLoader;
} LIBMATTI_FML_StartupArgs;

void LIBMATTI_FML_StartupArgs_Free(LIBMATTI_FML_StartupArgs *startupArgs);

#endif //MATTICRAFT_FML_STARTUPARGS_H
