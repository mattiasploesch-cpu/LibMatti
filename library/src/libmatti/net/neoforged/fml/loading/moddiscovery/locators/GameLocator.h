// Port of net.neoforged.fml.loading.moddiscovery.locators.GameLocator.
// detectMinecraftVersion() parses version.json through the ported Gson.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_GAMELOCATOR_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_GAMELOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

// Java: public GameLocator implements IModFileCandidateLocator
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator locator;
} LIBMATTI_FML_GameLocator;

// Java: public GameLocator()
LIBMATTI_FML_GameLocator *LIBMATTI_FML_GameLocator_New(void);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_GameLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                             LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
// Java: private static String detectMinecraftVersion(JarContents mcJarContents) - NULL when version.json is missing
char *LIBMATTI_FML_GameLocator_DetectMinecraftVersion(LIBMATTI_FML_JarContents *mcJarContents);
// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_GameLocator_GetPriority(void *self);
// Java: @Override public String toString() { return "game locator"; }
char *LIBMATTI_FML_GameLocator_ToString(void *self);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_GAMELOCATOR_H
