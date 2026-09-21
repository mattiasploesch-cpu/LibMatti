#include "libmatti/net/neoforged/neoforgespi/locating/IDependencyLocator.h"

// Java: public void scanMods(List<IModFile> loadedMods, IDiscoveryPipeline pipeline)
void LIBMATTI_NEOFORGESPI_IDependencyLocator_ScanMods(const LIBMATTI_NEOFORGESPI_IDependencyLocator *locator,
                                                      LIBMATTI_NEOFORGESPI_IModFile **loadedMods,
                                                      size_t loadedModCount,
                                                      LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline)
{
    locator->scanMods(locator->ordered.self, loadedMods, loadedModCount, pipeline);
}
