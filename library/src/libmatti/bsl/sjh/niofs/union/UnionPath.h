//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.niofs.union.UnionPath (java.nio.file.Path).

#ifndef MATTICRAFT_UNIONPATH_H
#define MATTICRAFT_UNIONPATH_H

#include "libmatti/java/net/URI.h"

#include <stddef.h>

// Java: class UnionPath implements Path
typedef struct LIBMATTI_UNION_UnionFileSystem LIBMATTI_UNION_UnionFileSystem;

typedef struct LIBMATTI_UNION_UnionPath
{
    LIBMATTI_UNION_UnionFileSystem *fileSystem;
    int absolute;
    char **pathParts; // String[] pathParts
    size_t partCount;
    struct LIBMATTI_UNION_UnionPath *normalized; // cache, computed once (Java: field)
} LIBMATTI_UNION_UnionPath;

// Java: UnionPath(UnionFileSystem, String... pathParts) - splits/joins like Java
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_New(LIBMATTI_UNION_UnionFileSystem *fileSystem, const char **pathParts, size_t partCount);
void LIBMATTI_UNION_UnionPath_Free(LIBMATTI_UNION_UnionPath *path);

LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionPath_GetFileSystem(const LIBMATTI_UNION_UnionPath *path);
int LIBMATTI_UNION_UnionPath_IsAbsolute(const LIBMATTI_UNION_UnionPath *path);
// Java: getRoot() - the filesystem root
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetRoot(const LIBMATTI_UNION_UnionPath *path);
// Java: getFileName() - new path with the last part; empty path for no parts
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetFileName(const LIBMATTI_UNION_UnionPath *path);
// Java: getParent() - NULL if no parts
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetParent(const LIBMATTI_UNION_UnionPath *path);
size_t LIBMATTI_UNION_UnionPath_GetNameCount(const LIBMATTI_UNION_UnionPath *path);
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_GetName(const LIBMATTI_UNION_UnionPath *path, size_t index);
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Subpath(const LIBMATTI_UNION_UnionPath *path, size_t beginIndex, size_t endIndex);
int LIBMATTI_UNION_UnionPath_StartsWith(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other);
int LIBMATTI_UNION_UnionPath_EndsWith(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other);
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Normalize(const LIBMATTI_UNION_UnionPath *path);
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Resolve(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other);
LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionPath_Relativize(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other);
// Java: toString() - joins the parts with '/', prefixed with '/' if absolute
char *LIBMATTI_UNION_UnionPath_ToString(const LIBMATTI_UNION_UnionPath *path);
int LIBMATTI_UNION_UnionPath_Equals(const LIBMATTI_UNION_UnionPath *path, const LIBMATTI_UNION_UnionPath *other);
// Java: toUri() - "union:" + key + "!" + absolute path
LIBMATTI_JN_URI *LIBMATTI_UNION_UnionPath_ToUri(const LIBMATTI_UNION_UnionPath *path);

#endif //MATTICRAFT_UNIONPATH_H