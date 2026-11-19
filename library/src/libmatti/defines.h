




#ifndef MATTICRAFT_DEFINES_H
#define MATTICRAFT_DEFINES_H

#include <stddef.h>

typedef struct
{
    char *name;
    char *location;
} LIBMATTI_BSL_BoostrapLauncher_Module;
























typedef struct
{
    char *filename;
    char *moduleNumber;
} LIBMATTI_BSL_FilenameMapEntry;

typedef struct
{
    LIBMATTI_BSL_FilenameMapEntry *entries;
    size_t count;
} LIBMATTI_BSL_FilenameMap;



typedef struct
{
    char *path;
    char *jarname;
} LIBMATTI_BSL_PathLookupEntry;

typedef struct
{
    LIBMATTI_BSL_PathLookupEntry *entries;
    size_t count;
} LIBMATTI_BSL_PathLookup;



typedef struct
{
    char *name;
    char **paths;
    size_t pathCount;
} LIBMATTI_BSL_MergeMapEntry;

typedef struct
{
    LIBMATTI_BSL_MergeMapEntry *entries;
    size_t count;
} LIBMATTI_BSL_MergeMap;

// Java: record PackageTracker(Set<String> packages, Path... paths)
//       implements UnionPathFilter
// The path filter decides which paths may be included in a module: paths whose
// package is already claimed by a previous module are filtered out.
typedef struct
{
    char **packages;

    size_t packageCount;
    char **paths;

    size_t pathCount;
} LIBMATTI_BSL_PackageTracker;

#define LIBMATTI_MC_VERSION_STRING "1.21.12"

#endif
