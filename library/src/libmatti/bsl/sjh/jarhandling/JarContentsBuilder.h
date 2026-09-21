//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.JarContentsBuilder.

#ifndef MATTICRAFT_JARCONTENTSBUILDER_H
#define MATTICRAFT_JARCONTENTSBUILDER_H

#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPathFilter.h"
#include "libmatti/java/util/jar/Manifest.h"

#include <stddef.h>

// Java: final class JarContentsBuilder
typedef struct
{
    char **paths;
    size_t pathCount;
    // Java: Supplier<Manifest> defaultManifest - takes ownership of the given manifest
    LIBMATTI_JU_Manifest *defaultManifest;
    LIBMATTI_UNION_UnionPathFilter *pathFilter; // NULL = no filter
} LIBMATTI_JH_JarContentsBuilder;

LIBMATTI_JH_JarContentsBuilder *LIBMATTI_JH_JarContentsBuilder_New(void);

// Java: paths(Path...)
void LIBMATTI_JH_JarContentsBuilder_Paths(LIBMATTI_JH_JarContentsBuilder *builder, const char **paths, size_t pathCount);
// Java: defaultManifest(Supplier<Manifest>) - the builder takes ownership
void LIBMATTI_JH_JarContentsBuilder_DefaultManifest(LIBMATTI_JH_JarContentsBuilder *builder, LIBMATTI_JU_Manifest *manifest);
// Java: pathFilter(UnionPathFilter)
void LIBMATTI_JH_JarContentsBuilder_PathFilter(LIBMATTI_JH_JarContentsBuilder *builder, LIBMATTI_UNION_UnionPathFilter *pathFilter);

// Java: build() - returns NULL on failure; the default manifest ownership is transferred
LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContentsBuilder_Build(LIBMATTI_JH_JarContentsBuilder *builder);
void LIBMATTI_JH_JarContentsBuilder_Free(LIBMATTI_JH_JarContentsBuilder *builder);

#endif //MATTICRAFT_JARCONTENTSBUILDER_H