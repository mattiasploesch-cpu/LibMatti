// Port of net.neoforged.neoforgespi.ILaunchContext.

#ifndef MATTICRAFT_NEOFORGESPI_ILAUNCHCONTEXT_H
#define MATTICRAFT_NEOFORGESPI_ILAUNCHCONTEXT_H

#include "libmatti/java/util/ServiceLoader.h"
#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/fml/loading/VersionInfo.h"

#include <stddef.h>

// Java: public interface ILaunchContext
// The C port represents the interface as a vtable; 'self' is the implementing object.
typedef struct LIBMATTI_NEOFORGESPI_ILaunchContext LIBMATTI_NEOFORGESPI_ILaunchContext;

struct LIBMATTI_NEOFORGESPI_ILaunchContext
{
    void *self;

    // Java: Dist getRequiredDistribution()
    LIBMATTI_DIST_Dist (*getRequiredDistribution)(void *self);
    // Java: Path gameDirectory()
    const char *(*gameDirectory)(void *self);
    // Java: <T> Stream<ServiceLoader.Provider<T>> loadServices(Class<T> serviceClass)
    LIBMATTI_JU_ServiceLoader *(*loadServices)(void *self, const char *serviceClass);
    // Java: boolean isLocated(Path path)
    int (*isLocated)(void *self, const char *path);
    // Java: boolean addLocated(Path path)
    int (*addLocated)(void *self, const char *path);
    // Java: VersionInfo getVersions()
    LIBMATTI_FML_VersionInfo *(*getVersions)(void *self);
};

// Java: public Dist getRequiredDistribution()
LIBMATTI_DIST_Dist LIBMATTI_NEOFORGESPI_ILaunchContext_GetRequiredDistribution(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context);
// Java: public Path gameDirectory()
const char *LIBMATTI_NEOFORGESPI_ILaunchContext_GameDirectory(const LIBMATTI_NEOFORGESPI_ILaunchContext *context);
// Java: public <T> Stream<ServiceLoader.Provider<T>> loadServices(Class<T> serviceClass)
LIBMATTI_JU_ServiceLoader *LIBMATTI_NEOFORGESPI_ILaunchContext_LoadServices(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *serviceClass);
// Java: public boolean isLocated(Path path)
int LIBMATTI_NEOFORGESPI_ILaunchContext_IsLocated(const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *path);
// Java: public boolean addLocated(Path path)
int LIBMATTI_NEOFORGESPI_ILaunchContext_AddLocated(const LIBMATTI_NEOFORGESPI_ILaunchContext *context, const char *path);
// Java: public VersionInfo getVersions()
LIBMATTI_FML_VersionInfo *LIBMATTI_NEOFORGESPI_ILaunchContext_GetVersions(
    const LIBMATTI_NEOFORGESPI_ILaunchContext *context);

#endif //MATTICRAFT_NEOFORGESPI_ILAUNCHCONTEXT_H
