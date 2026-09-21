//
// Port of cpw.mods.modlauncher.api.NamedPath.
//

#ifndef MATTICRAFT_MODLAUNCHER_NAMEDPATH_H
#define MATTICRAFT_MODLAUNCHER_NAMEDPATH_H

#include <stddef.h>

// Java: record NamedPath(String name, Path... paths)
typedef struct
{
    char *name;
    char **paths;
    size_t pathCount;
} LIBMATTI_MLA_NamedPath;

#endif //MATTICRAFT_MODLAUNCHER_NAMEDPATH_H
