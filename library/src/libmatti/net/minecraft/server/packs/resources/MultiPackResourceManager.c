// Port of net.minecraft.server.packs.resources.MultiPackResourceManager and
// net.minecraft.server.packs.resources.ReloadableResourceManager.

#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// ReloadInstance
// ---------------------------------------------------------------------------

LIBMATTI_MC_ReloadInstance *LIBMATTI_MC_ReloadInstance_Simple(void)
{
    LIBMATTI_MC_ReloadInstance *instance = calloc(1, sizeof(LIBMATTI_MC_ReloadInstance));
    instance->done = 1;
    instance->progress = 1.0f;
    return instance;
}

void LIBMATTI_MC_ReloadInstance_Free(LIBMATTI_MC_ReloadInstance *instance)
{
    free(instance);
}

// ---------------------------------------------------------------------------
// MultiPackResourceManager
// ---------------------------------------------------------------------------

LIBMATTI_MC_MultiPackResourceManager *LIBMATTI_MC_MultiPackResourceManager_New(int type,
                                                                                LIBMATTI_MC_PackResources *const *packs,
                                                                                size_t packCount)
{
    LIBMATTI_MC_MultiPackResourceManager *manager = calloc(1, sizeof(LIBMATTI_MC_MultiPackResourceManager));
    manager->type = type;
    manager->packs = packCount > 0 ? malloc(packCount * sizeof(LIBMATTI_MC_PackResources *)) : NULL;
    if (packCount > 0) memcpy(manager->packs, packs, packCount * sizeof(LIBMATTI_MC_PackResources *));
    manager->packCount = packCount;

    // Java: for each namespace in the union of the pack namespaces, build one
    // FallbackResourceManager and push every pack (in order)
    size_t capacity = 0;
    for (size_t i = 0; i < packCount; i++)
    {
        size_t count = 0;
        char **namespaces = packs[i]->vtable->getNamespaces(packs[i], type, &count);
        if (namespaces == NULL) continue;
        for (size_t j = 0; j < count; j++)
        {
            int duplicate = 0;
            for (size_t k = 0; k < manager->managerCount; k++)
            {
                if (strcmp(manager->namespaces[k], namespaces[j]) == 0)
                {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate)
            {
                free(namespaces[j]);
                continue;
            }
            if (manager->managerCount == capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                manager->namespaces =
                    realloc(manager->namespaces, capacity * sizeof(char *));
                manager->managers = realloc(manager->managers, capacity * sizeof(LIBMATTI_MC_FallbackResourceManager *));
            }
            manager->namespaces[manager->managerCount] = namespaces[j];
            manager->managers[manager->managerCount] =
                LIBMATTI_MC_FallbackResourceManager_New(type, namespaces[j]);
            manager->managerCount++;
        }
        free(namespaces);
    }

    // Java: namespaces.forEach((name, manager) -> packs.forEach(manager::add))
    for (size_t i = 0; i < packCount; i++)
    {
        for (size_t j = 0; j < manager->managerCount; j++)
        {
            LIBMATTI_MC_FallbackResourceManager_Push(manager->managers[j], packs[i]);
        }
    }

    return manager;
}

static const LIBMATTI_MC_FallbackResourceManager *find_manager(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                               const char *namespace)
{
    for (size_t i = 0; i < manager->managerCount; i++)
    {
        if (strcmp(manager->namespaces[i], namespace) == 0) return manager->managers[i];
    }
    return NULL;
}

LIBMATTI_MC_Resource *LIBMATTI_MC_MultiPackResourceManager_GetResource(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                                       const char *namespace, const char *path)
{
    const LIBMATTI_MC_FallbackResourceManager *fallback = find_manager(manager, namespace);
    if (fallback == NULL) return NULL;
    return LIBMATTI_MC_FallbackResourceManager_GetResource(fallback, namespace, path);
}

LIBMATTI_MC_Resource **LIBMATTI_MC_MultiPackResourceManager_GetResourceStack(
    const LIBMATTI_MC_MultiPackResourceManager *manager, const char *namespace, const char *path, size_t *outCount)
{
    *outCount = 0;
    const LIBMATTI_MC_FallbackResourceManager *fallback = find_manager(manager, namespace);
    if (fallback == NULL) return NULL;
    return LIBMATTI_MC_FallbackResourceManager_GetResourceStack(fallback, namespace, path, outCount);
}

void LIBMATTI_MC_MultiPackResourceManager_ListResources(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                        const char *prefix, void *userData,
                                                        LIBMATTI_MC_ResourceOutput onResource)
{
    for (size_t i = 0; i < manager->managerCount; i++)
    {
        LIBMATTI_MC_FallbackResourceManager_ListResources(manager->managers[i], prefix, userData, onResource);
    }
}

void LIBMATTI_MC_MultiPackResourceManager_ListResourceStacks(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                             const char *prefix, void *userData,
                                                             LIBMATTI_MC_ResourceOutput onResource)
{
    for (size_t i = 0; i < manager->managerCount; i++)
    {
        LIBMATTI_MC_FallbackResourceManager_ListResourceStacks(manager->managers[i], prefix, userData, onResource);
    }
}

char **LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(const LIBMATTI_MC_MultiPackResourceManager *manager,
                                                          size_t *outCount)
{
    char **names = malloc((manager->managerCount + 1) * sizeof(char *));
    for (size_t i = 0; i < manager->managerCount; i++)
        names[i] = strdup(manager->namespaces[i]);
    names[manager->managerCount] = NULL;
    *outCount = manager->managerCount;
    return names;
}

LIBMATTI_MC_PackResources *const *LIBMATTI_MC_MultiPackResourceManager_ListPacks(
    const LIBMATTI_MC_MultiPackResourceManager *manager, size_t *outCount)
{
    *outCount = manager->packCount;
    return (LIBMATTI_MC_PackResources *const *) manager->packs;
}

void LIBMATTI_MC_MultiPackResourceManager_Free(LIBMATTI_MC_MultiPackResourceManager *manager)
{
    for (size_t i = 0; i < manager->managerCount; i++)
    {
        free(manager->namespaces[i]);
        LIBMATTI_MC_FallbackResourceManager_Free(manager->managers[i]);
    }
    free(manager->namespaces);
    free(manager->managers);
    // Java: the manager does not own the packs; the repository closes them
    free(manager->packs);
    free(manager);
}

// ---------------------------------------------------------------------------
// ReloadableResourceManager
// ---------------------------------------------------------------------------

LIBMATTI_MC_ReloadableResourceManager *LIBMATTI_MC_ReloadableResourceManager_New(int type)
{
    LIBMATTI_MC_ReloadableResourceManager *manager = calloc(1, sizeof(LIBMATTI_MC_ReloadableResourceManager));
    manager->type = type;
    return manager;
}

void LIBMATTI_MC_ReloadableResourceManager_Add(LIBMATTI_MC_ReloadableResourceManager *manager, void *listener)
{
    manager->listeners = realloc(manager->listeners, (manager->listenerCount + 1) * sizeof(void *));
    manager->listeners[manager->listenerCount] = listener;
    manager->listenerCount++;
}

LIBMATTI_MC_ReloadInstance *LIBMATTI_MC_ReloadableResourceManager_CreateReload(
    LIBMATTI_MC_ReloadableResourceManager *manager)
{
    // Java: the reload pipeline runs the listeners async; the port's listeners are
    // data-only placeholders, so the reload completes immediately
    if (manager->reloadInstance != NULL) LIBMATTI_MC_ReloadInstance_Free(manager->reloadInstance);
    manager->reloadInstance = LIBMATTI_MC_ReloadInstance_Simple();
    return manager->reloadInstance;
}

LIBMATTI_MC_Resource *LIBMATTI_MC_ReloadableResourceManager_GetResource(
    const LIBMATTI_MC_ReloadableResourceManager *manager, const char *namespace, const char *path)
{
    if (manager->resources == NULL) return NULL;
    return LIBMATTI_MC_MultiPackResourceManager_GetResource(manager->resources, namespace, path);
}

void LIBMATTI_MC_ReloadableResourceManager_ListResources(const LIBMATTI_MC_ReloadableResourceManager *manager,
                                                         const char *prefix, void *userData,
                                                         LIBMATTI_MC_ResourceOutput onResource)
{
    if (manager->resources == NULL) return;
    LIBMATTI_MC_MultiPackResourceManager_ListResources(manager->resources, prefix, userData, onResource);
}

char **LIBMATTI_MC_ReloadableResourceManager_GetNamespaces(const LIBMATTI_MC_ReloadableResourceManager *manager,
                                                           size_t *outCount)
{
    if (manager->resources == NULL)
    {
        char **names = malloc(sizeof(char *));
        names[0] = NULL;
        *outCount = 0;
        return names;
    }
    return LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(manager->resources, outCount);
}

void LIBMATTI_MC_ReloadableResourceManager_Free(LIBMATTI_MC_ReloadableResourceManager *manager)
{
    if (manager->resources != NULL) LIBMATTI_MC_MultiPackResourceManager_Free(manager->resources);
    if (manager->reloadInstance != NULL) LIBMATTI_MC_ReloadInstance_Free(manager->reloadInstance);
    free(manager->listeners);
    free(manager);
}
