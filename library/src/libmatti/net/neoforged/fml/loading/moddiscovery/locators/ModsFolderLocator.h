// Port of net.neoforged.fml.loading.moddiscovery.locators.ModsFolderLocator.
// The loader reads native mods, so the scanned suffix is ".so" (see EarlyServiceDiscovery).

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MODSFOLDERLOCATOR_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MODSFOLDERLOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

// Java: private static final String SUFFIX = ".jar"
#define LIBMATTI_FML_MODS_FOLDER_SUFFIX ".so"

// Java: public class ModsFolderLocator implements IModFileCandidateLocator
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator locator;

    const char *modFolder;
    const char *customName;
} LIBMATTI_FML_ModsFolderLocator;

// Java: public ModsFolderLocator() - the mods folder from FMLPaths
LIBMATTI_FML_ModsFolderLocator *LIBMATTI_FML_ModsFolderLocator_New(void);
// Java: public ModsFolderLocator(Path modFolder, String name)
LIBMATTI_FML_ModsFolderLocator *LIBMATTI_FML_ModsFolderLocator_NewWithFolder(const char *modFolder,
                                                                             const char *name);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_ModsFolderLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                   LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
// Java: @Override public String toString() { return "{" + customName + " locator at " + this.modFolder + "}"; }
char *LIBMATTI_FML_ModsFolderLocator_ToString(void *self);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_MODSFOLDERLOCATOR_H
