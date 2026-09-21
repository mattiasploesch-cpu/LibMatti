// Port of cpw.mods.modlauncher.serviceapi.ITransformerDiscoveryService.

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMERDISCOVERYSERVICE_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMERDISCOVERYSERVICE_H

#include "libmatti/cpw/modlauncher/api/NamedPath.h"

#include <stddef.h>

// Java: interface ITransformerDiscoveryService
typedef struct LIBMATTI_MLS_ITransformerDiscoveryService
{
    // Java: List<NamedPath> candidates(Path gameDirectory)
    LIBMATTI_MLA_NamedPath *(*candidates)(struct LIBMATTI_MLS_ITransformerDiscoveryService *self,
                                          const char *gameDirectory, size_t *count);
    // Java: default List<NamedPath> candidates(Path gameDirectory, String launchTarget)
    LIBMATTI_MLA_NamedPath *(*candidatesWithTarget)(struct LIBMATTI_MLS_ITransformerDiscoveryService *self,
                                                    const char *gameDirectory, const char *launchTarget,
                                                    size_t *count);
    // Java: default void earlyInitialization(String launchTarget, String[] arguments)
    void (*earlyInitialization)(struct LIBMATTI_MLS_ITransformerDiscoveryService *self, const char *launchTarget,
                                int argc, char *argv[]);
} LIBMATTI_MLS_ITransformerDiscoveryService;

// Java: default List<NamedPath> candidates(Path gameDirectory, String launchTarget) { return candidates(gameDirectory); }
LIBMATTI_MLA_NamedPath *LIBMATTI_MLS_ITransformerDiscoveryService_Candidates(
    LIBMATTI_MLS_ITransformerDiscoveryService *service, const char *gameDirectory, const char *launchTarget,
    size_t *count);
// Java: default void earlyInitialization(String launchTarget, String[] arguments) {}
void LIBMATTI_MLS_ITransformerDiscoveryService_EarlyInitialization(
    LIBMATTI_MLS_ITransformerDiscoveryService *service, const char *launchTarget, int argc, char *argv[]);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMERDISCOVERYSERVICE_H
