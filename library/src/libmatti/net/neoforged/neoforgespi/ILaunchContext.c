#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"

// Java: public Dist getRequiredDistribution()
LIBMATTI_DIST_Dist LIBMATTI_NEOFORGESPI_ILaunchContext_GetRequiredDistribution(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context)
{
    return context->getRequiredDistribution(context->self);
}

// Java: public Path gameDirectory()
const char *LIBMATTI_NEOFORGESPI_ILaunchContext_GameDirectory(const LIBMATTI_NEOFORGESPI_ILaunchContext *context)
{
    return context->gameDirectory(context->self);
}

// Java: public <T> Stream<ServiceLoader.Provider<T>> loadServices(Class<T> serviceClass)
LIBMATTI_JU_ServiceLoader *LIBMATTI_NEOFORGESPI_ILaunchContext_LoadServices(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass)
{
    return context->loadServices(context->self, serviceClass);
}

// Java: public boolean isLocated(Path path)
int LIBMATTI_NEOFORGESPI_ILaunchContext_IsLocated(const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *path)
{
    return context->isLocated(context->self, path);
}

// Java: public boolean addLocated(Path path)
int LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *path)
{
    return context->addLocated(context->self, path);
}

// Java: public VersionInfo getVersions()
LIBMATTI_FML_VersionInfo *LIBMATTI_NEOFORGESPI_ILaunchContext_GetVersions(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context)
{
    return context->getVersions(context->self);
}
