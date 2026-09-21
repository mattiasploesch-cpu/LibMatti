//
// Created by administrator on 09.09.26.
//

#ifndef MATTICRAFT_DEFINES_H
#define MATTICRAFT_DEFINES_H

#include <stddef.h>

typedef struct
{
    char *name;
    char *location;
} LIBMATTI_BSL_BoostrapLauncher_Module;

// ---------------------------------------------------------------------------
// Java -> C mapping for the records/classes used by
// cpw.mods.bootstraplauncher.BootstrapLauncher:
//
//   record ModuleWithLocation(String name, Path location)
//                                   -> LIBMATTI_BSL_BoostrapLauncher_Module
//   record PackageTracker(Set<String> packages, Path... paths)
//                                   -> LIBMATTI_BSL_PackageTracker
//   Map<String, String> filenameMap -> LIBMATTI_BSL_FilenameMap
//   Map<Path, String> pathLookup    -> LIBMATTI_BSL_PathLookup
//   Map<String, List<Path>> mergeMap -> LIBMATTI_BSL_MergeMap
//   SecureJar (cpw jarhandling, external lib in Java)
//                                   -> LIBMATTI_JH_SecureJar
//                                      (bsl/sjh/jarhandling/SecureJar.h)
// ---------------------------------------------------------------------------

// --- BootstrapLauncher data structures (mirror BootstrapLauncher.java) ---

// Map: filename -> module number ("0", "1", ...), built from the mergeModules
// property (see getMergeFilenameMap in BootstrapLauncher.java)
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

// Map: path -> jarname, filled via computeIfAbsent in the main classpath loop
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

// Map: jarname -> list of paths that are combined into that module
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
    char **packages;      // Set<String> packages (copied, like Set.copyOf)
    size_t packageCount;
    char **paths;         // Path... paths (part of the record; unused in test())
    size_t pathCount;
} LIBMATTI_BSL_PackageTracker;

#define LIBMATTI_MC_VERSION_STRING "1.21.12"

#endif //MATTICRAFT_DEFINES_H