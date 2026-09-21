// Port of net.minecraft.server.packs.CompositePackResources.

#include "libmatti/net/minecraft/server/packs/CompositePackResources.h"

#include <stdlib.h>
#include <string.h>

static unsigned char *composite_get_root_resource(LIBMATTI_MC_PackResources *pack, const char *const *paths,
                                                  size_t pathCount, size_t *outLength)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    *outLength = 0;
    // Java: the first stack that can serve the resource wins
    for (size_t i = 0; i < self->stackCount; i++)
    {
        unsigned char *data = self->stacks[i]->vtable->getRootResource(self->stacks[i], paths, pathCount, outLength);
        if (data != NULL) return data;
    }
    return NULL;
}

static unsigned char *composite_open(LIBMATTI_MC_PackResources *pack, int type, const char *namespace,
                                     const char *path, size_t *outLength)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    *outLength = 0;
    for (size_t i = 0; i < self->stackCount; i++)
    {
        unsigned char *data = self->stacks[i]->vtable->open(self->stacks[i], type, namespace, path, outLength);
        if (data != NULL) return data;
    }
    return NULL;
}

static unsigned char *composite_open_resource(LIBMATTI_MC_PackResources *pack, int type, const char *namespace,
                                              const char *path, size_t *outLength)
{
    return composite_open(pack, type, namespace, path, outLength);
}

static void composite_list_resources(LIBMATTI_MC_PackResources *pack, int type, const char *namespace,
                                     const char *prefix, void *userData,
                                     void (*onFile)(void *, const char *, const char *, int, size_t))
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    for (size_t i = 0; i < self->stackCount; i++)
    {
        self->stacks[i]->vtable->listResources(self->stacks[i], type, namespace, prefix, userData, onFile);
    }
}

static char **composite_get_namespaces(LIBMATTI_MC_PackResources *pack, int type, size_t *outCount)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    // Java: union of the stack namespaces
    char **names = NULL;
    size_t count = 0;
    size_t capacity = 0;
    for (size_t i = 0; i < self->stackCount; i++)
    {
        size_t stackCount = 0;
        char **stackNames = self->stacks[i]->vtable->getNamespaces(self->stacks[i], type, &stackCount);
        if (stackNames == NULL) continue;
        for (size_t j = 0; j < stackCount; j++)
        {
            int duplicate = 0;
            for (size_t k = 0; k < count; k++)
            {
                if (strcmp(names[k], stackNames[j]) == 0)
                {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate)
            {
                free(stackNames[j]);
                continue;
            }
            if (count == capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                names = realloc(names, capacity * sizeof(char *));
            }
            names[count] = stackNames[j];
            count++;
        }
        free(stackNames);
    }
    names = realloc(names, (count + 1) * sizeof(char *));
    names[count] = NULL;
    *outCount = count;
    return names;
}

static void *composite_get_metadata_section(LIBMATTI_MC_PackResources *pack,
                                            const LIBMATTI_MC_MetadataSectionSerializer *serializer, char **outError)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    for (size_t i = 0; i < self->stackCount; i++)
    {
        void *section = self->stacks[i]->vtable->getMetadataSection(self->stacks[i], serializer, outError);
        if (section != NULL || *outError != NULL) return section;
    }
    return NULL;
}

static const char *composite_pack_id(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    return self->location->id;
}

static LIBMATTI_MC_PackLocationInfo *composite_location(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    return self->location;
}

static void composite_close(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_CompositePackResources *self = (LIBMATTI_MC_CompositePackResources *) pack;
    for (size_t i = 0; i < self->stackCount; i++)
        self->stacks[i]->vtable->close(self->stacks[i]);
}

static const LIBMATTI_MC_PackResources_VTable COMPOSITE_VTABLE = {
    composite_get_root_resource, composite_open,     composite_open_resource, composite_list_resources,
    composite_get_namespaces,    composite_get_metadata_section, composite_pack_id, composite_location,
    composite_close,
};

LIBMATTI_MC_CompositePackResources *LIBMATTI_MC_CompositePackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                           LIBMATTI_MC_PackResources *const *stacks,
                                                                           size_t stackCount)
{
    LIBMATTI_MC_CompositePackResources *self = calloc(1, sizeof(LIBMATTI_MC_CompositePackResources));
    self->vtable = &COMPOSITE_VTABLE;
    self->location = location;
    self->stacks = stackCount > 0 ? malloc(stackCount * sizeof(LIBMATTI_MC_PackResources *)) : NULL;
    memcpy(self->stacks, stacks, stackCount * sizeof(LIBMATTI_MC_PackResources *));
    self->stackCount = stackCount;
    return self;
}

void LIBMATTI_MC_CompositePackResources_Free(LIBMATTI_MC_CompositePackResources *pack)
{
    free(pack->stacks);
    LIBMATTI_MC_PackLocationInfo_Free(pack->location);
    free(pack);
}
