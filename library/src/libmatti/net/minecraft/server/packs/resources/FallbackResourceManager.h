// Port of net.minecraft.server.packs.resources.FallbackResourceManager.

#ifndef MATTICRAFT_MC_PACKS_RESOURCES_FALLBACKRESOURCEMANAGER_H
#define MATTICRAFT_MC_PACKS_RESOURCES_FALLBACKRESOURCEMANAGER_H

#include "libmatti/net/minecraft/server/packs/PackResources.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/resources/Resource.h"

#include <stddef.h>

// Java: the resource output callback (file entries only; directories are skipped
// by the port's walkers before the callback fires)
typedef void (*LIBMATTI_MC_ResourceOutput)(void *userData, const char *namespace, const char *path, int hasResource,
                                           size_t resourceLength);

typedef struct LIBMATTI_MC_FallbackResourceManager
{
    // Java: private final PackType packType
    int packType;
    // Java: private final String namespace
    char *namespace;
    // Java: protected final List<PackResources> packs (fallback order: later packs win)
    LIBMATTI_MC_PackResources **packs;
    size_t packCount;
} LIBMATTI_MC_FallbackResourceManager;

// Java: public FallbackResourceManager(PackType packType, String namespace)
LIBMATTI_MC_FallbackResourceManager *LIBMATTI_MC_FallbackResourceManager_New(int packType, const char *namespace);

// Java: public void push(PackResources pack) / add (the top of the stack wins)
void LIBMATTI_MC_FallbackResourceManager_Push(LIBMATTI_MC_FallbackResourceManager *manager,
                                              LIBMATTI_MC_PackResources *pack);
// Java: public void pushFilter(PackResources pack) - filter section handled internally
void LIBMATTI_MC_FallbackResourceManager_PushFilter(LIBMATTI_MC_FallbackResourceManager *manager,
                                                    LIBMATTI_MC_PackResources *pack);

// Java: public Optional<Resource> getResource(ResourceLocation)
LIBMATTI_MC_Resource *LIBMATTI_MC_FallbackResourceManager_GetResource(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                                      const char *namespace, const char *path);
// Java: public List<Resource> getResourceStack(ResourceLocation)
LIBMATTI_MC_Resource **LIBMATTI_MC_FallbackResourceManager_GetResourceStack(
    const LIBMATTI_MC_FallbackResourceManager *manager, const char *namespace, const char *path, size_t *outCount);
// Java: public Map<ResourceLocation, Resource> listResources(String path, Predicate<ResourceLocation> filter)
// The port walks with a prefix and reports through the callback.
void LIBMATTI_MC_FallbackResourceManager_ListResources(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                       const char *prefix, void *userData,
                                                       LIBMATTI_MC_ResourceOutput onResource);
// Java: public Map<ResourceLocation, IoSupplier<InputStream>> listResourceStacks(...)
void LIBMATTI_MC_FallbackResourceManager_ListResourceStacks(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                            const char *prefix, void *userData,
                                                            LIBMATTI_MC_ResourceOutput onResource);
// Java: public Set<String> getNamespaces()
char **LIBMATTI_MC_FallbackResourceManager_GetNamespaces(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                         size_t *outCount);

void LIBMATTI_MC_FallbackResourceManager_Free(LIBMATTI_MC_FallbackResourceManager *manager);

#endif
