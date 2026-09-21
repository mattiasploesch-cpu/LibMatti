// Port of net.minecraft.server.packs.metadata.MetadataSectionSerializer (the
// registry key) and net.minecraft.server.packs.metadata.pack.PackMetadataSection.
//
// Java registers the serializers in PackMetadataSection's static TYPE plus the
// game's serializer registry; the port uses one shared map keyed by the section
// name (e.g. "pack", "filter", "feature_flags", "overlays").

#ifndef MATTICRAFT_MC_PACKS_METADATA_METADATASECTION_H
#define MATTICRAFT_MC_PACKS_METADATA_METADATASECTION_H

#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/net/minecraft/server/packs/repository/PackFormat.h"

#include <stddef.h>

typedef struct LIBMATTI_MC_MetadataSectionSerializer
{
    const char *name;
    // Java: T fromJson(JsonElement) - returns malloc'd section, NULL + *outError on failure
    void *(*fromJson)(const struct LIBMATTI_MC_MetadataSectionSerializer *self,
                      const LIBMATTI_GSON_JsonElement *element, char **outError);
} LIBMATTI_MC_MetadataSectionSerializer;

// Java: PackMetadataSection.TYPE - "pack"
const LIBMATTI_MC_MetadataSectionSerializer *LIBMATTI_MC_PackMetadataSection_TYPE(void);

typedef struct LIBMATTI_MC_PackMetadataSection
{
    const char *description;
    LIBMATTI_MC_PackFormat_IntermediaryFormat descriptionFormat;
} LIBMATTI_MC_PackMetadataSection;

LIBMATTI_MC_PackMetadataSection *LIBMATTI_MC_PackMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError);
void LIBMATTI_MC_PackMetadataSection_Free(LIBMATTI_MC_PackMetadataSection *section);

// Java: ResourceFilterSection ("filter")
typedef struct LIBMATTI_MC_ResourceFilterSection_Pattern
{
    char *namespace;
    char *path;
} LIBMATTI_MC_ResourceFilterSection_Pattern;

typedef struct LIBMATTI_MC_ResourceFilterSection
{
    LIBMATTI_MC_ResourceFilterSection_Pattern *patterns;
    size_t patternCount;
} LIBMATTI_MC_ResourceFilterSection;

LIBMATTI_MC_ResourceFilterSection *LIBMATTI_MC_ResourceFilterSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError);
void LIBMATTI_MC_ResourceFilterSection_Free(LIBMATTI_MC_ResourceFilterSection *section);

// Java: FeatureFlagsMetadataSection ("feature_flags")
typedef struct LIBMATTI_MC_FeatureFlagsMetadataSection
{
    char **flags;
    size_t flagCount;
} LIBMATTI_MC_FeatureFlagsMetadataSection;

LIBMATTI_MC_FeatureFlagsMetadataSection *LIBMATTI_MC_FeatureFlagsMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError);
void LIBMATTI_MC_FeatureFlagsMetadataSection_Free(LIBMATTI_MC_FeatureFlagsMetadataSection *section);

// Java: OverlayMetadataSection ("overlays")
typedef struct LIBMATTI_MC_OverlayMetadataSection
{
    char **directories;
    LIBMATTI_MC_PackFormat_IntermediaryFormat *formats;
    size_t entryCount;
} LIBMATTI_MC_OverlayMetadataSection;

LIBMATTI_MC_OverlayMetadataSection *LIBMATTI_MC_OverlayMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError);
void LIBMATTI_MC_OverlayMetadataSection_Free(LIBMATTI_MC_OverlayMetadataSection *section);

// Java: static void registerSerializers() side of the metadata registry
void LIBMATTI_MC_MetadataSection_RegisterSerializers(void);
// Java: Map<String, MetadataSectionSerializer> - shared, lazily built
const LIBMATTI_MC_MetadataSectionSerializer *LIBMATTI_MC_MetadataSection_Get(const char *name);

#endif
