// Port of net.minecraft.server.packs.repository.Pack.

#include "libmatti/net/minecraft/server/packs/repository/Pack.h"

#include <stdlib.h>
#include <string.h>

Pack *LIBMATTI_MC_Pack_Create(LIBMATTI_MC_PackLocationInfo *location,
                              const LIBMATTI_MC_Pack_ResourcesSupplier *supplier,
                              LIBMATTI_MC_PackMetadataSection *metadata, LIBMATTI_MC_PackSelectionConfig config)
{
    Pack *pack = calloc(1, sizeof(Pack));
    pack->location = location;
    pack->resourcesSupplier = supplier;
    pack->metadata = metadata;
    pack->selectionConfig = config;
    return pack;
}

Pack *LIBMATTI_MC_Pack_ReadMetaAndCreate(LIBMATTI_MC_PackLocationInfo *location,
                                         const LIBMATTI_MC_Pack_ResourcesSupplier *supplier,
                                         LIBMATTI_MC_InclusiveRange supportedFormats,
                                         LIBMATTI_MC_PackSelectionConfig config)
{
    // Java: try (PackResources packresources = supplier.open(location)) {
    //           PackMetadataSection metadata = packresources.getMetadataSection(PackMetadataSection.TYPE)
    //               .orElseThrow(() -> new IllegalArgumentException(...));
    LIBMATTI_MC_PackResources *resources = supplier->open(supplier, location);
    if (resources == NULL)
    {
        LIBMATTI_MC_PackLocationInfo_Free(location);
        return NULL;
    }

    char *error = NULL;
    LIBMATTI_MC_PackMetadataSection *metadata = (LIBMATTI_MC_PackMetadataSection *) resources->vtable->getMetadataSection(
        resources, LIBMATTI_MC_PackMetadataSection_TYPE(), &error);
    resources->vtable->close(resources);
    free(error);

    if (metadata == NULL)
    {
        // Java: orElseThrow IllegalArgumentException("Missing metadata for pack %s".formatted(id))
        LIBMATTI_MC_PackLocationInfo_Free(location);
        return NULL;
    }

    // Java: IntermediaryFormat.build from the supportedFormats fallback when missing
    if (!metadata->descriptionFormat.hasSupported && !metadata->descriptionFormat.hasMin)
    {
        metadata->descriptionFormat.hasSupported = 1;
        metadata->descriptionFormat.supported = supportedFormats;
    }

    return LIBMATTI_MC_Pack_Create(location, supplier, metadata, config);
}

const char *LIBMATTI_MC_Pack_GetId(const Pack *pack)
{
    return pack->location->id;
}

LIBMATTI_MC_PackLocationInfo *LIBMATTI_MC_Pack_Location(const Pack *pack)
{
    return pack->location;
}

const LIBMATTI_MC_PackMetadataSection *LIBMATTI_MC_Pack_GetMetadata(const Pack *pack)
{
    return pack->metadata;
}

int LIBMATTI_MC_Pack_IsRequired(const Pack *pack)
{
    return pack->selectionConfig.required;
}

int LIBMATTI_MC_Pack_IsFixedPosition(const Pack *pack)
{
    return pack->selectionConfig.fixedPosition;
}

LIBMATTI_MC_PackPosition LIBMATTI_MC_Pack_GetFixedPosition(const Pack *pack)
{
    // Java: selectionConfig.fixedPosition(); the port stores the position flag
    return LIBMATTI_MC_PackPosition_ONLY_FIXED;
}

int LIBMATTI_MC_Pack_IsHidden(const Pack *pack)
{
    return pack->selectionConfig.hidden;
}

LIBMATTI_MC_PackResources *LIBMATTI_MC_Pack_Open(const Pack *pack)
{
    return pack->resourcesSupplier->open(pack->resourcesSupplier, pack->location);
}

LIBMATTI_MC_PackResources *LIBMATTI_MC_Pack_OpenFull(const Pack *pack, LIBMATTI_MC_PackResources *primary)
{
    // Java: openFull(PackResources primary) returns primary when the pack has no extras
    return primary;
}

int LIBMATTI_MC_Pack_CompareTo(const Pack *a, const Pack *b)
{
    // Java: COMPARATOR = comparing(Pack::getPackSource::ordering).thenComparing(Pack::getId)
    const LIBMATTI_MC_PackSource *sourceA = a->location->source;
    const LIBMATTI_MC_PackSource *sourceB = b->location->source;
    if (sourceA->type != sourceB->type) return sourceA->type < sourceB->type ? -1 : 1;
    return strcmp(a->location->id, b->location->id);
}

void LIBMATTI_MC_Pack_Free(Pack *pack)
{
    LIBMATTI_MC_PackMetadataSection_Free(pack->metadata);
    LIBMATTI_MC_PackLocationInfo_Free(pack->location);
    free(pack);
}
