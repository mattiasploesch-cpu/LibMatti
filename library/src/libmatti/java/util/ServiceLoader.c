// Port of java.util.ServiceLoader.

#include "libmatti/java/util/ServiceLoader.h"

#include <stdlib.h>
#include <string.h>

// Java: the module system's service registry; the host fills it in.
static LIBMATTI_JU_ServiceLoader_Provider *registry = NULL;
static size_t registryCount = 0;

void LIBMATTI_JU_ServiceLoader_Register(const char *serviceType, void *provider, const char *providerClassName,
                                        const char *location)
{
    registry = realloc(registry, sizeof(*registry) * (registryCount + 1));
    registry[registryCount].serviceType = serviceType;
    registry[registryCount].provider = provider;
    registry[registryCount].providerClassName = providerClassName;
    registry[registryCount].location = location;
    registryCount++;
}

static LIBMATTI_JU_ServiceLoader *load_for(const char *serviceType)
{
    LIBMATTI_JU_ServiceLoader *loader = calloc(1, sizeof(LIBMATTI_JU_ServiceLoader));
    loader->serviceType = serviceType;

    for (size_t i = 0; i < registryCount; i++)
    {
        if (strcmp(registry[i].serviceType, serviceType) != 0) continue;

        loader->providers = realloc(loader->providers, sizeof(*loader->providers) * (loader->count + 1));
        loader->providers[loader->count++] = registry[i];
    }

    return loader;
}

LIBMATTI_JU_ServiceLoader *LIBMATTI_JU_ServiceLoader_Load(const void *layer, const char *serviceType)
{
    // Java: the layer restricts which modules may contribute providers; the C port
    // has a single registry, so the layer is not used to filter.
    (void)layer;
    return load_for(serviceType);
}

LIBMATTI_JU_ServiceLoader *LIBMATTI_JU_ServiceLoader_LoadFrom(const char *serviceType)
{
    return load_for(serviceType);
}

// Java: ServiceLoaderUtils.fileNameFor(Class<?>) resolves the module location
const char *LIBMATTI_JU_ServiceLoader_LocationForClass(const char *providerClassName)
{
    for (size_t i = 0; i < registryCount; i++)
    {
        if (registry[i].providerClassName == NULL) continue;
        if (strcmp(registry[i].providerClassName, providerClassName) != 0) continue;
        return registry[i].location;
    }
    return NULL;
}

LIBMATTI_JU_ServiceLoader_Provider *LIBMATTI_JU_ServiceLoader_Stream(const LIBMATTI_JU_ServiceLoader *loader,
                                                                     size_t *count)
{
    *count = loader->count;
    return loader->providers;
}

void LIBMATTI_JU_ServiceLoader_Free(LIBMATTI_JU_ServiceLoader *loader)
{
    if (loader == NULL) return;
    free(loader->providers);
    free(loader);
}
