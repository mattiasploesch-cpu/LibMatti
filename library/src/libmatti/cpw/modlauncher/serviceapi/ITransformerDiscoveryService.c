// Port of cpw.mods.modlauncher.serviceapi.ITransformerDiscoveryService.

#include "libmatti/cpw/modlauncher/serviceapi/ITransformerDiscoveryService.h"

LIBMATTI_MLA_NamedPath *LIBMATTI_MLS_ITransformerDiscoveryService_Candidates(
    LIBMATTI_MLS_ITransformerDiscoveryService *service, const char *gameDirectory, const char *launchTarget,
    size_t *count)
{
    // Java: default List<NamedPath> candidates(Path, String) { return candidates(gameDirectory); }
    if (service->candidatesWithTarget != NULL)
        return service->candidatesWithTarget(service, gameDirectory, launchTarget, count);
    if (service->candidates != NULL) return service->candidates(service, gameDirectory, count);

    *count = 0;
    return NULL;
}

void LIBMATTI_MLS_ITransformerDiscoveryService_EarlyInitialization(
    LIBMATTI_MLS_ITransformerDiscoveryService *service, const char *launchTarget, int argc, char *argv[])
{
    // Java: default void earlyInitialization(String, String[]) {}
    if (service->earlyInitialization != NULL)
        service->earlyInitialization(service, launchTarget, argc, argv);
}
