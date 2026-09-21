//
// Created by administrator on 09.09.26.
//

#include "JarContentsBuilder.h"

#include "libmatti/bsl/sjh/jarhandling/impl/JarContentsImpl.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JH_JarContentsBuilder *LIBMATTI_JH_JarContentsBuilder_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JH_JarContentsBuilder));
}

void LIBMATTI_JH_JarContentsBuilder_Paths(LIBMATTI_JH_JarContentsBuilder *builder, const char **paths, size_t pathCount)
{
    for (size_t i = 0; i < builder->pathCount; i++) free(builder->paths[i]);
    free(builder->paths);

    builder->paths = calloc(pathCount, sizeof(char *));
    for (size_t i = 0; i < pathCount; i++) builder->paths[i] = strdup(paths[i]);
    builder->pathCount = pathCount;
}

void LIBMATTI_JH_JarContentsBuilder_DefaultManifest(LIBMATTI_JH_JarContentsBuilder *builder, LIBMATTI_JU_Manifest *manifest)
{
    LIBMATTI_JU_Manifest_Free(builder->defaultManifest);
    builder->defaultManifest = manifest;
}

void LIBMATTI_JH_JarContentsBuilder_PathFilter(LIBMATTI_JH_JarContentsBuilder *builder, LIBMATTI_UNION_UnionPathFilter *pathFilter)
{
    builder->pathFilter = pathFilter;
}

LIBMATTI_JH_JarContents *LIBMATTI_JH_JarContentsBuilder_Build(LIBMATTI_JH_JarContentsBuilder *builder)
{
    LIBMATTI_JH_JarContents *contents = (LIBMATTI_JH_JarContents *)LIBMATTI_JH_JarContentsImpl_New(
        (const char **)builder->paths, builder->pathCount, builder->defaultManifest, builder->pathFilter);

    // the default manifest ownership was transferred to the contents
    builder->defaultManifest = NULL;

    return contents;
}

void LIBMATTI_JH_JarContentsBuilder_Free(LIBMATTI_JH_JarContentsBuilder *builder)
{
    if (builder == NULL) return;

    for (size_t i = 0; i < builder->pathCount; i++) free(builder->paths[i]);
    free(builder->paths);
    LIBMATTI_JU_Manifest_Free(builder->defaultManifest);
    free(builder);
}