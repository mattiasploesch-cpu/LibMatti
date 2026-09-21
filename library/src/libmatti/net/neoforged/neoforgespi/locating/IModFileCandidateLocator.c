#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/locators/ModsFolderLocator.h"

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_FindCandidates(
    const LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    locator->findCandidates(locator->ordered.self, context, pipeline);
}

// Java: locator.toString()
char *LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ToString(
    const LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator)
{
    return locator->toString(locator->ordered.self);
}

// Java: static IModFileCandidateLocator forFolder(File folder, String identifier)
LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ForFolder(const char *folder,
                                                                                                      const char *identifier)
{
    // Java: return new ModsFolderLocator(folder.toPath(), identifier);
    LIBMATTI_FML_ModsFolderLocator *modsFolderLocator = LIBMATTI_FML_ModsFolderLocator_NewWithFolder(folder, identifier);
    return &modsFolderLocator->locator;
}
