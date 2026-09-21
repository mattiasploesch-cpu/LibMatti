// Port of net.minecraft.server.packs.resources.Resource,
// net.minecraft.server.packs.resources.ResourceMetadata and
// net.minecraft.server.packs.resources.ResourceFilterSection (the filter gate
// lives in FallbackResourceManager).

#ifndef MATTICRAFT_MC_PACKS_RESOURCES_RESOURCE_H
#define MATTICRAFT_MC_PACKS_RESOURCES_RESOURCE_H

#include <stddef.h>
#include <stdint.h>

// Java: interface IoSupplier<InputStream> - the port materializes the bytes
typedef struct LIBMATTI_MC_Resource_IoSupplier
{
    unsigned char *bytes;
    size_t length;
} LIBMATTI_MC_Resource_IoSupplier;

// Java: record Resource(PackResources source, IoSupplier<InputStream> streamSupplier,
//       IoSupplier<ResourceMetadata> metadataSupplier)
typedef struct LIBMATTI_MC_Resource
{
    // Java: the owning pack (used for the source pack name)
    void *source;
    LIBMATTI_MC_Resource_IoSupplier streamSupplier;
    // Java: ResourceMetadata.EMPTY when no .mcmeta exists
    int hasMetadata;
    unsigned char *metadataBytes;
    size_t metadataLength;
} LIBMATTI_MC_Resource;

// Java: Resource create(PackResources, IoSupplier<InputStream>) - metadata empty
LIBMATTI_MC_Resource *LIBMATTI_MC_Resource_Create(void *source, unsigned char *bytes, size_t length);
// Java: Resource create(PackResources, IoSupplier, IoSupplier<ResourceMetadata>)
LIBMATTI_MC_Resource *LIBMATTI_MC_Resource_CreateWithMetadata(void *source, unsigned char *bytes, size_t length,
                                                              unsigned char *metadataBytes, size_t metadataLength);
// Java: public InputStream open()
unsigned char *LIBMATTI_MC_Resource_Open(const LIBMATTI_MC_Resource *resource, size_t *outLength);
// Java: public ResourceMetadata metadata()
const unsigned char *LIBMATTI_MC_Resource_Metadata(const LIBMATTI_MC_Resource *resource, size_t *outLength);
// Java: public String sourcePackId()
const char *LIBMATTI_MC_Resource_SourcePackId(const LIBMATTI_MC_Resource *resource);

void LIBMATTI_MC_Resource_Free(LIBMATTI_MC_Resource *resource);

#endif
