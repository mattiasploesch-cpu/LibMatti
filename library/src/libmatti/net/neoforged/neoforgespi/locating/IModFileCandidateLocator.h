// Port of net.neoforged.neoforgespi.locating.IModFileCandidateLocator.
// TODO: net.neoforged.fml.loading.moddiscovery.locators.ModsFolderLocator - forFolder()

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILECANDIDATELOCATOR_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILECANDIDATELOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IOrderedProvider.h"

// Java: public interface IModFileCandidateLocator extends IOrderedProvider
typedef struct LIBMATTI_NEOFORGESPI_IModFileCandidateLocator
{
    // Java: extends IOrderedProvider
    LIBMATTI_NEOFORGESPI_IOrderedProvider ordered;

    // Java: void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
    void (*findCandidates)(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                           LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);

    // Java: String toString() - caller frees the result
    char *(*toString)(void *self);
} LIBMATTI_NEOFORGESPI_IModFileCandidateLocator;

// Java: locator.toString()
char *LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ToString(
    const LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_FindCandidates(
    const LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);

// Java: static IModFileCandidateLocator forFolder(File folder, String identifier)
// TODO: net.neoforged.fml.loading.moddiscovery.locators.ModsFolderLocator
LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ForFolder(const char *folder,
                                                                                                      const char *identifier);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILECANDIDATELOCATOR_H
