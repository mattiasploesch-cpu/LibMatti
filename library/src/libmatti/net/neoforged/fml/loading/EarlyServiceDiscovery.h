// Port of net.neoforged.fml.loading.EarlyServiceDiscovery.
// The collaborators (JarModuleInfo, IModFile.create, JarModsDotTomlModFileReader,
// ModFile) are all ported.

#ifndef MATTICRAFT_FML_LOADING_EARLYSERVICEDISCOVERY_H
#define MATTICRAFT_FML_LOADING_EARLYSERVICEDISCOVERY_H

#include "libmatti/net/neoforged/fml/startup/StartupArgs.h"

#include <stddef.h>

// Java: public static List<ModFile> findEarlyServiceJars(StartupArgs startupArgs, Path directory)
void **LIBMATTI_FML_EarlyServiceDiscovery_FindEarlyServiceJars(LIBMATTI_FML_StartupArgs *startupArgs,
                                                               const char *directory, size_t *count);

#endif //MATTICRAFT_FML_LOADING_EARLYSERVICEDISCOVERY_H
