// Port of net.minecraft.server.packs.resources.MultiPackResourceManager,
// net.minecraft.server.packs.resources.ReloadableResourceManager and
// net.minecraft.server.packs.resources.ReloadInstance.

#ifndef MATTICRAFT_MC_PACKS_RESOURCES_MULTIPACKRESOURCEMANAGER_H
#define MATTICRAFT_MC_PACKS_RESOURCES_MULTIPACKRESOURCEMANAGER_H

#include "libmatti/net/minecraft/server/packs/PackResources.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/resources/FallbackResourceManager.h"
#include "libmatti/net/minecraft/server/packs/resources/Resource.h"

#include <stddef.h>

// ---------------------------------------------------------------------------
// ReloadInstance (Java: interface, the port is a completion counter)
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_MC_ReloadInstance
{
    int done;
    int failures;
    // Java: float getActualProgress()
    float progress;
} LIBMATTI_MC_ReloadInstance;

LIBMATTI_MC_ReloadInstance *LIBMATTI_MC_ReloadInstance_Simple(void);
void LIBMATTI_MC_ReloadInstance_Free(LIBMATTI_MC_ReloadInstance *instance);

// ---------------------------------------------------------------------------
// ResourceManager (Java: interface -> the port's concrete MultiPackResourceManager)
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_MC_MultiPackResourceManager
{
    // Java: private final PackType type
    int type;
    // Java: private final List<PackResources> packs
    LIBMATTI_MC_PackResources **packs;
    size_t packCount;
    // Java: private final Map<String, FallbackResourceManager> namespacedManagers
    char **namespaces;
    LIBMATTI_MC_FallbackResourceManager **managers;
    size_t managerCount;
} LIBMATTI_MC_MultiPackResourceManager;

// Java: public MultiPackResourceManager(PackType type, List<PackResources> packs)
LIBMATTI_MC_MultiPackResourceManager *LIBMATTI_MC_MultiPackResourceManager_New(int type,
                                                                                LIBMATTI_MC_PackResources *const *packs,
                                                                                size_t packCount);

// Java: Optional<Resource> getResource(ResourceLocation)
LIBMATTI_MC_Resource *LIBMATTI_MC_MultiPackResourceManager_GetResource(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                                       const char *namespace, const char *path);
// Java: List<Resource> getResourceStack(ResourceLocation)
LIBMATTI_MC_Resource **LIBMATTI_MC_MultiPackResourceManager_GetResourceStack(
    const LIBMATTI_MC_MultiPackResourceManager *manager, const char *namespace, const char *path, size_t *outCount);
// Java: Map<ResourceLocation, Resource> listResources(String path, Predicate)
void LIBMATTI_MC_MultiPackResourceManager_ListResources(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                        const char *prefix, void *userData,
                                                        LIBMATTI_MC_ResourceOutput onResource);
// Java: Map<ResourceLocation, IoSupplier<InputStream>> listResourceStacks(String path, Predicate)
void LIBMATTI_MC_MultiPackResourceManager_ListResourceStacks(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                             const char *prefix, void *userData,
                                                             LIBMATTI_MC_ResourceOutput onResource);
// Java: Set<String> getNamespaces()
char **LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                          size_t *outCount);
// Java: Stream<PackResources> listPacks()
LIBMATTI_MC_PackResources *const *LIBMATTI_MC_MultiPackResourceManager_ListPacks(
    const LIBMATTI_MC_MultiPackResourceManager *manager, size_t *outCount);

void LIBMATTI_MC_MultiPackResourceManager_Free(LIBMATTI_MC_MultiPackResourceManager *manager);

// ---------------------------------------------------------------------------
// ReloadableResourceManager (Java: implements ResourceManager by delegation)
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_MC_ReloadableResourceManager
{
    // Java: private final PackType type
    int type;
    // Java: private final List<PreparableReloadListener> listeners
    void **listeners;
    size_t listenerCount;
    // Java: private MultiPackResourceManager resources (the current instance)
    LIBMATTI_MC_MultiPackResourceManager *resources;
    // Java: private ReloadInstance reloadInstance
    LIBMATTI_MC_ReloadInstance *reloadInstance;
} LIBMATTI_MC_ReloadableResourceManager;

LIBMATTI_MC_ReloadableResourceManager *LIBMATTI_MC_ReloadableResourceManager_New(int type);
// Java: public void add(PreparableReloadListener listener)
void LIBMATTI_MC_ReloadableResourceManager_Add(LIBMATTI_MC_ReloadableResourceManager *manager, void *listener);
// Java: public ReloadInstance createReload(Executor, Executor, CompletableFuture<Unit>)
// The port runs the (empty) reload synchronously.
LIBMATTI_MC_ReloadInstance *LIBMATTI_MC_ReloadableResourceManager_CreateReload(
    LIBMATTI_MC_ReloadableResourceManager *manager);
// Delegated ResourceManager surface
LIBMATTI_MC_Resource *LIBMATTI_MC_ReloadableResourceManager_GetResource(
    const LIBMATTI_MC_ReloadableResourceManager *manager, const char *namespace, const char *path);
void LIBMATTI_MC_ReloadableResourceManager_ListResources(const LIBMATTI_MC_ReloadableResourceManager *manager,
                                                         const char *prefix, void *userData,
                                                         LIBMATTI_MC_ResourceOutput onResource);
char **LIBMATTI_MC_ReloadableResourceManager_GetNamespaces(const LIBMATTI_MC_ReloadableResourceManager *manager,
                                                           size_t *outCount);

void LIBMATTI_MC_ReloadableResourceManager_Free(LIBMATTI_MC_ReloadableResourceManager *manager);

#endif
