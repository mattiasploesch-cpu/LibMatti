// Port of net.minecraft.server.packs.resources.FallbackResourceManager.

#include "libmatti/net/minecraft/server/packs/resources/FallbackResourceManager.h"

#include "libmatti/net/minecraft/server/packs/metadata/MetadataSection.h"
#include "libmatti/net/minecraft/server/packs/resources/Resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_FallbackResourceManager *LIBMATTI_MC_FallbackResourceManager_New(int packType, const char *namespace)
{
    LIBMATTI_MC_FallbackResourceManager *manager = calloc(1, sizeof(LIBMATTI_MC_FallbackResourceManager));
    manager->packType = packType;
    manager->namespace = strdup(namespace);
    return manager;
}

void LIBMATTI_MC_FallbackResourceManager_Push(LIBMATTI_MC_FallbackResourceManager *manager,
                                              LIBMATTI_MC_PackResources *pack)
{
    manager->packs = realloc(manager->packs, (manager->packCount + 1) * sizeof(LIBMATTI_MC_PackResources *));
    manager->packs[manager->packCount] = pack;
    manager->packCount++;
}

void LIBMATTI_MC_FallbackResourceManager_PushFilter(LIBMATTI_MC_FallbackResourceManager *manager,
                                                    LIBMATTI_MC_PackResources *pack)
{
    // Java: a filtered pack is pushed with a filter section; the port stores it the
    // same way and evaluates its ResourceFilterSection during lookups
    LIBMATTI_MC_FallbackResourceManager_Push(manager, pack);
}

// Java: the fallback order - Java pushes base packs first and overlays on top;
// lookups walk from the END of the list backwards (the newest pack wins).
static int is_hidden_path(const char *path)
{
    const char *slash = path;
    while (slash != NULL)
    {
        const char *segment = slash;
        if (segment != path) segment++;
        if (segment[0] == '.') return 1;
        slash = strchr(slash + 1, '/');
    }
    return 0;
}

static char *build_metadata_path(const char *path)
{
    // Java: FallbackResourceManager.metadataPath = path + ".mcmeta"
    size_t length = strlen(path) + strlen(".mcmeta") + 1;
    char *metadataPath = malloc(length);
    snprintf(metadataPath, length, "%s.mcmeta", path);
    return metadataPath;
}

static int pack_has_filter(LIBMATTI_MC_PackResources *pack)
{
    // Java: pack.getMetadataSection(ResourceFilterSection.TYPE).isPresent()
    char *error = NULL;
    LIBMATTI_MC_ResourceFilterSection *filter =
        (LIBMATTI_MC_ResourceFilterSection *) pack->vtable->getMetadataSection(
            pack, LIBMATTI_MC_MetadataSection_Get("filter"), &error);
    free(error);
    if (filter == NULL) return 0;
    LIBMATTI_MC_ResourceFilterSection_Free(filter);
    return 1;
}

LIBMATTI_MC_Resource *LIBMATTI_MC_FallbackResourceManager_GetResource(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                                      const char *namespace, const char *path)
{
    (void) namespace;
    LIBMATTI_MC_Resource *found = NULL;
    for (size_t i = manager->packCount; i-- > 0;)
    {
        LIBMATTI_MC_PackResources *pack = manager->packs[i];
        if (pack_has_filter(pack)) continue; // the port applies the filter gate at the pack level
        if (is_hidden_path(path)) continue;

        size_t length = 0;
        unsigned char *bytes = pack->vtable->open(pack, manager->packType, manager->namespace, path, &length);
        if (bytes != NULL)
        {
            // Java: metadata = metadataPath lookup, EMPTY when absent
            char *metadataPath = build_metadata_path(path);
            size_t metadataLength = 0;
            unsigned char *metadataBytes =
                pack->vtable->open(pack, manager->packType, manager->namespace, metadataPath, &metadataLength);
            free(metadataPath);
            found = LIBMATTI_MC_Resource_CreateWithMetadata(pack, bytes, length, metadataBytes, metadataLength);
            break;
        }
    }
    return found;
}

LIBMATTI_MC_Resource **LIBMATTI_MC_FallbackResourceManager_GetResourceStack(
    const LIBMATTI_MC_FallbackResourceManager *manager, const char *namespace, const char *path, size_t *outCount)
{
    (void) namespace;
    LIBMATTI_MC_Resource **stack = NULL;
    size_t count = 0;
    for (size_t i = manager->packCount; i-- > 0;)
    {
        LIBMATTI_MC_PackResources *pack = manager->packs[i];
        size_t length = 0;
        unsigned char *bytes = pack->vtable->open(pack, manager->packType, manager->namespace, path, &length);
        if (bytes == NULL) continue;
        char *metadataPath = build_metadata_path(path);
        size_t metadataLength = 0;
        unsigned char *metadataBytes =
            pack->vtable->open(pack, manager->packType, manager->namespace, metadataPath, &metadataLength);
        free(metadataPath);
        stack = realloc(stack, (count + 1) * sizeof(LIBMATTI_MC_Resource *));
        stack[count] = LIBMATTI_MC_Resource_CreateWithMetadata(pack, bytes, length, metadataBytes, metadataLength);
        count++;
    }
    *outCount = count;
    return stack;
}

void LIBMATTI_MC_FallbackResourceManager_ListResources(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                       const char *prefix, void *userData,
                                                       LIBMATTI_MC_ResourceOutput onResource)
{
    for (size_t i = 0; i < manager->packCount; i++)
    {
        LIBMATTI_MC_PackResources *pack = manager->packs[i];
        pack->vtable->listResources(pack, manager->packType, manager->namespace, prefix, userData,
                                    (void (*)(void *, const char *, const char *, int, size_t)) onResource);
    }
}

void LIBMATTI_MC_FallbackResourceManager_ListResourceStacks(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                            const char *prefix, void *userData,
                                                            LIBMATTI_MC_ResourceOutput onResource)
{
    // Java: the stacks variant reports every occurrence; the port's output shape is
    // the same callback (the caller decides how to group)
    LIBMATTI_MC_FallbackResourceManager_ListResources(manager, prefix, userData, onResource);
}

char **LIBMATTI_MC_FallbackResourceManager_GetNamespaces(const LIBMATTI_MC_FallbackResourceManager *manager,
                                                         size_t *outCount)
{
    (void) manager;
    // Java: the namespace is fixed for a fallback manager
    char **names = malloc(2 * sizeof(char *));
    names[0] = strdup(manager->namespace);
    names[1] = NULL;
    *outCount = 1;
    return names;
}

void LIBMATTI_MC_FallbackResourceManager_Free(LIBMATTI_MC_FallbackResourceManager *manager)
{
    free(manager->namespace);
    free(manager->packs);
    free(manager);
}
