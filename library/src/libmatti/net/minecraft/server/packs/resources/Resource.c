// Port of net.minecraft.server.packs.resources.Resource.

#include "libmatti/net/minecraft/server/packs/resources/Resource.h"

#include "libmatti/net/minecraft/server/packs/PackResources.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_Resource *LIBMATTI_MC_Resource_CreateWithMetadata(void *source, unsigned char *bytes, size_t length,
                                                              unsigned char *metadataBytes, size_t metadataLength)
{
    LIBMATTI_MC_Resource *resource = calloc(1, sizeof(LIBMATTI_MC_Resource));
    resource->source = source;
    resource->streamSupplier.bytes = bytes;
    resource->streamSupplier.length = length;
    if (metadataBytes != NULL)
    {
        resource->hasMetadata = 1;
        resource->metadataBytes = metadataBytes;
        resource->metadataLength = metadataLength;
    }
    return resource;
}

LIBMATTI_MC_Resource *LIBMATTI_MC_Resource_Create(void *source, unsigned char *bytes, size_t length)
{
    return LIBMATTI_MC_Resource_CreateWithMetadata(source, bytes, length, NULL, 0);
}

unsigned char *LIBMATTI_MC_Resource_Open(const LIBMATTI_MC_Resource *resource, size_t *outLength)
{
    *outLength = resource->streamSupplier.length;
    return resource->streamSupplier.bytes;
}

const unsigned char *LIBMATTI_MC_Resource_Metadata(const LIBMATTI_MC_Resource *resource, size_t *outLength)
{
    *outLength = resource->hasMetadata ? resource->metadataLength : 0;
    return resource->hasMetadata ? resource->metadataBytes : NULL;
}

const char *LIBMATTI_MC_Resource_SourcePackId(const LIBMATTI_MC_Resource *resource)
{
    // Java: resource.source.packId()
    LIBMATTI_MC_PackResources *pack = (LIBMATTI_MC_PackResources *) resource->source;
    return pack->vtable->packId(pack);
}

void LIBMATTI_MC_Resource_Free(LIBMATTI_MC_Resource *resource)
{
    free(resource->streamSupplier.bytes);
    free(resource->metadataBytes);
    free(resource);
}
