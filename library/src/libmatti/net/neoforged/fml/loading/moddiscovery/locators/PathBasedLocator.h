// Port of net.neoforged.fml.loading.moddiscovery.locators.PathBasedLocator.
// "Locates" mods from a fixed set of paths.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_PATHBASEDLOCATOR_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_PATHBASEDLOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

#include <stddef.h>

// Java: public record PathBasedLocator(String name, List<Path> paths) implements IModFileCandidateLocator
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator locator;

    char *name;
    const char **paths;
    size_t pathCount;
} LIBMATTI_FML_PathBasedLocator;

// Java: public PathBasedLocator(String name, Path... paths)
LIBMATTI_FML_PathBasedLocator *LIBMATTI_FML_PathBasedLocator_New(const char *name, const char **paths,
                                                                 size_t pathCount);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_PathBasedLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                 LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_PathBasedLocator_GetPriority(void *self);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_PATHBASEDLOCATOR_H
