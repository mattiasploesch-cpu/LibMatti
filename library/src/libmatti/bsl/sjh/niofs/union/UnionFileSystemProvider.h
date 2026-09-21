//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.niofs.union.UnionFileSystemProvider.

#ifndef MATTICRAFT_UNIONFILESYSTEMPROVIDER_H
#define MATTICRAFT_UNIONFILESYSTEMPROVIDER_H

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystem.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPathFilter.h"

#include <stddef.h>

// Java: class UnionFileSystemProvider extends FileSystemProvider
typedef struct LIBMATTI_UNION_UnionFileSystemProvider
{
    // Java: Map<String, UnionFileSystem> fileSystems
    char **keys;
    LIBMATTI_UNION_UnionFileSystem **fileSystems;
    size_t count;
    int index;
} LIBMATTI_UNION_UnionFileSystemProvider;

// Java: installedProviders() singleton lookup for the "union" scheme
LIBMATTI_UNION_UnionFileSystemProvider *LIBMATTI_UNION_UnionFileSystemProvider_Get(void);

// Java: newFileSystem(@Nullable UnionPathFilter, Path... paths) - NULL on failure
LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionFileSystemProvider_NewFileSystem(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                                                     LIBMATTI_UNION_UnionPathFilter *pathFilter,
                                                                                     const char **paths, size_t pathCount);

// Java: removeFileSystem(UnionFileSystem)
void LIBMATTI_UNION_UnionFileSystemProvider_RemoveFileSystem(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                             LIBMATTI_UNION_UnionFileSystem *fileSystem);

// Java: getScheme()
const char *LIBMATTI_UNION_UnionFileSystemProvider_GetScheme(void);

#endif //MATTICRAFT_UNIONFILESYSTEMPROVIDER_H