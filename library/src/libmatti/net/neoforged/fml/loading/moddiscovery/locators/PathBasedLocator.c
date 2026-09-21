#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/PathBasedLocator.h"

#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"

#include <stdlib.h>
#include <string.h>

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_PathBasedLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                 LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    LIBMATTI_FML_PathBasedLocator *locator = self;
    (void) context;

    // Java: for (var path : paths) pipeline.addPath(path, ModFileDiscoveryAttributes.DEFAULT, IncompatibleFileReporting.ERROR);
    for (size_t i = 0; i < locator->pathCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
            LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
        LIBMATTI_NEOFORGESPI_IModFile *added = NULL;
        LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
            pipeline, locator->paths[i], &attributes, LIBMATTI_NEOFORGESPI_IncompatibleFileReporting_ERROR, &added);
    }
}

// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_PathBasedLocator_GetPriority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_HIGHEST_SYSTEM_PRIORITY;
}

// Java: public PathBasedLocator(String name, Path... paths)
LIBMATTI_FML_PathBasedLocator *LIBMATTI_FML_PathBasedLocator_New(const char *name, const char **paths,
                                                                 size_t pathCount)
{
    LIBMATTI_FML_PathBasedLocator *locator = calloc(1, sizeof(LIBMATTI_FML_PathBasedLocator));
    locator->name = strdup(name);
    locator->paths = malloc(sizeof(char *) * (pathCount > 0 ? pathCount : 1));
    for (size_t i = 0; i < pathCount; i++) locator->paths[i] = strdup(paths[i]);
    locator->pathCount = pathCount;

    locator->locator.ordered.self = locator;
    locator->locator.ordered.getPriority = LIBMATTI_FML_PathBasedLocator_GetPriority;
    locator->locator.findCandidates = LIBMATTI_FML_PathBasedLocator_FindCandidates;
    return locator;
}
