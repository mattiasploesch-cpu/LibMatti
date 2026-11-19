





#ifndef MATTICRAFT_JARCONTENTSIMPL_H
#define MATTICRAFT_JARCONTENTSIMPL_H

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

// Java: class JarContentsImpl implements JarContents
typedef struct
{
    LIBMATTI_UNION_UnionFileSystem *filesystem;
    char **validPaths;

    size_t validPathCount;
    LIBMATTI_JU_Manifest *manifest;
// TODO: signingData (JarSigningData) and nameOverrides (multi-release jars)
    char **packages;

    size_t packageCount;
    LIBMATTI_JH_Provider *providers;

    size_t providerCount;
} LIBMATTI_JH_JarContentsImpl;

// Java: JarContentsImpl(Path[] paths, Supplier<Manifest> defaultManifest, UnionPathFilter pathFilter)
// Takes ownership of defaultManifest; returns NULL on failure (Java: throws).
LIBMATTI_JH_JarContentsImpl *LIBMATTI_JH_JarContentsImpl_New(const char **paths, size_t pathCount,
                                                             LIBMATTI_JU_Manifest *defaultManifest,
                                                             LIBMATTI_UNION_UnionPathFilter *pathFilter);

#endif
