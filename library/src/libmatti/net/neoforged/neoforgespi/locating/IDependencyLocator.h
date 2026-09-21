// Port of net.neoforged.neoforgespi.locating.IDependencyLocator.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IDEPENDENCYLOCATOR_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IDEPENDENCYLOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IOrderedProvider.h"

#include <stddef.h>

// Java: public interface IDependencyLocator extends IOrderedProvider
typedef struct LIBMATTI_NEOFORGESPI_IDependencyLocator
{
    // Java: extends IOrderedProvider
    LIBMATTI_NEOFORGESPI_IOrderedProvider ordered;

    // Java: void scanMods(List<IModFile> loadedMods, IDiscoveryPipeline pipeline)
    void (*scanMods)(void *self, LIBMATTI_NEOFORGESPI_IModFile **loadedMods, size_t loadedModCount,
                     LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
} LIBMATTI_NEOFORGESPI_IDependencyLocator;

// Java: public void scanMods(List<IModFile> loadedMods, IDiscoveryPipeline pipeline)
void LIBMATTI_NEOFORGESPI_IDependencyLocator_ScanMods(const LIBMATTI_NEOFORGESPI_IDependencyLocator *locator,
                                                      LIBMATTI_NEOFORGESPI_IModFile **loadedMods,
                                                      size_t loadedModCount,
                                                      LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IDEPENDENCYLOCATOR_H
