// Port of net.neoforged.fml.loading.moddiscovery.locators.InDevFolderLocator.
// Finds mods and services that are passed as exploded folders on the classpath and are grouped explicitly.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVFOLDERLOCATOR_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVFOLDERLOCATOR_H

#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

#include <stddef.h>

// Java: record VirtualJarManifestEntry(String name, List<File> files)
typedef struct
{
    char *name;
    char **files;
    size_t fileCount;
} LIBMATTI_FML_VirtualJarManifestEntry;

// Java: public class InDevFolderLocator implements IModFileCandidateLocator
typedef struct
{
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator locator;

    // Java: Map<File, VirtualJarManifestEntry> virtualJarMemberIndex
    char **memberKeys;
    LIBMATTI_FML_VirtualJarManifestEntry **memberValues;
    size_t memberCount;
} LIBMATTI_FML_InDevFolderLocator;

// Java: public InDevFolderLocator()
LIBMATTI_FML_InDevFolderLocator *LIBMATTI_FML_InDevFolderLocator_New(void);

// Java: public void findCandidates(ILaunchContext context, IDiscoveryPipeline pipeline)
void LIBMATTI_FML_InDevFolderLocator_FindCandidates(void *self, LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                    LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline);
// Java: private void loadFromSystemProperty()
void LIBMATTI_FML_InDevFolderLocator_LoadFromSystemProperty(LIBMATTI_FML_InDevFolderLocator *locator);
// Java: public int getPriority() { return HIGHEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_InDevFolderLocator_GetPriority(void *self);
// Java: @Override public String toString() { return "indevfolder"; }
char *LIBMATTI_FML_InDevFolderLocator_ToString(void *self);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_INDEVFOLDERLOCATOR_H
