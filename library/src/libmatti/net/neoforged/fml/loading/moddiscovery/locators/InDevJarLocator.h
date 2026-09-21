// Port of net.neoforged.fml.loading.moddiscovery.locators.InDevJarLocator.
// Finds mods and game libraries that are passed as files on the classpath.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVJARLOCATOR_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVJARLOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

// Java: public InDevJarLocator implements IModFileCandidateLocator
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator locator;
} LIBMATTI_FML_InDevJarLocator;

// Java: public InDevJarLocator()
LIBMATTI_FML_InDevJarLocator *LIBMATTI_FML_InDevJarLocator_New(void);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_InDevJarLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                 LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_InDevJarLocator_GetPriority(void *self);
// Java: @Override public String toString() { return "indevjar"; }
char *LIBMATTI_FML_InDevJarLocator_ToString(void *self);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVJARLOCATOR_H
