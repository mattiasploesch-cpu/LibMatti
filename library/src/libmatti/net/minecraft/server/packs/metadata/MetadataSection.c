// Port of the pack metadata sections: pack.PackMetadataSection,
// ResourceFilterSection, FeatureFlagsMetadataSection, OverlayMetadataSection,
// and the MetadataSectionSerializer registry (Java: the static TYPE fields plus
// PackMetadataSectionSerializer.SERIALIZERS registration via codec).

#include "libmatti/net/minecraft/server/packs/metadata/MetadataSection.h"

#include "libmatti/net/minecraft/server/packs/repository/PackFormat.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// The serializer registry (Java: built once per class initialization)
// ---------------------------------------------------------------------------

static LIBMATTI_MC_MetadataSectionSerializer *pack_serializer = NULL;
static LIBMATTI_MC_MetadataSectionSerializer *filter_serializer = NULL;
static LIBMATTI_MC_MetadataSectionSerializer *feature_flags_serializer = NULL;
static LIBMATTI_MC_MetadataSectionSerializer *overlay_serializer = NULL;

static char **registry_names = NULL;
static LIBMATTI_MC_MetadataSectionSerializer **registry_values = NULL;
static size_t registry_count = 0;
static size_t registry_capacity = 0;

static void registry_put(LIBMATTI_MC_MetadataSectionSerializer *serializer)
{
    if (registry_count == registry_capacity)
    {
        registry_capacity = registry_capacity == 0 ? 8 : registry_capacity * 2;
        registry_names = realloc(registry_names, registry_capacity * sizeof(char *));
        registry_values = realloc(registry_values, registry_capacity * sizeof(LIBMATTI_MC_MetadataSectionSerializer *));
    }
    registry_names[registry_count] = (char *) serializer->name;
    registry_values[registry_count] = serializer;
    registry_count++;
}

const LIBMATTI_MC_MetadataSectionSerializer *LIBMATTI_MC_MetadataSection_Get(const char *name)
{
    for (size_t i = 0; i < registry_count; i++)
    {
        if (strcmp(registry_names[i], name) == 0) return registry_values[i];
    }
    return NULL;
}

void LIBMATTI_MC_MetadataSection_RegisterSerializers(void)
{
    if (pack_serializer != NULL) return;

    pack_serializer = calloc(1, sizeof(LIBMATTI_MC_MetadataSectionSerializer));
    pack_serializer->name = "pack";
    pack_serializer->fromJson =
        (void *(*)(const LIBMATTI_MC_MetadataSectionSerializer *, const LIBMATTI_GSON_JsonElement *, char **))
            LIBMATTI_MC_PackMetadataSection_FromJson;

    filter_serializer = calloc(1, sizeof(LIBMATTI_MC_MetadataSectionSerializer));
    filter_serializer->name = "filter";
    filter_serializer->fromJson =
        (void *(*)(const LIBMATTI_MC_MetadataSectionSerializer *, const LIBMATTI_GSON_JsonElement *, char **))
            LIBMATTI_MC_ResourceFilterSection_FromJson;

    feature_flags_serializer = calloc(1, sizeof(LIBMATTI_MC_MetadataSectionSerializer));
    feature_flags_serializer->name = "feature_flags";
    feature_flags_serializer->fromJson =
        (void *(*)(const LIBMATTI_MC_MetadataSectionSerializer *, const LIBMATTI_GSON_JsonElement *, char **))
            LIBMATTI_MC_FeatureFlagsMetadataSection_FromJson;

    overlay_serializer = calloc(1, sizeof(LIBMATTI_MC_MetadataSectionSerializer));
    overlay_serializer->name = "overlays";
    overlay_serializer->fromJson =
        (void *(*)(const LIBMATTI_MC_MetadataSectionSerializer *, const LIBMATTI_GSON_JsonElement *, char **))
            LIBMATTI_MC_OverlayMetadataSection_FromJson;

    registry_put(pack_serializer);
    registry_put(filter_serializer);
    registry_put(feature_flags_serializer);
    registry_put(overlay_serializer);
}

const LIBMATTI_MC_MetadataSectionSerializer *LIBMATTI_MC_PackMetadataSection_TYPE(void)
{
    LIBMATTI_MC_MetadataSection_RegisterSerializers();
    return pack_serializer;
}

// ---------------------------------------------------------------------------
// pack.PackMetadataSection
// ---------------------------------------------------------------------------

LIBMATTI_MC_PackMetadataSection *LIBMATTI_MC_PackMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError)
{
    (void) self;
    *outError = NULL;

    // Java: record PackMetadataSection(Component description, int packFormat,
    //       Optional<InclusiveRange> supportedFormats, IntermediaryFormat descriptionFormat)
    LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_JsonElement_GetAsJsonObject((LIBMATTI_GSON_JsonElement *) element);
    if (object == NULL)
    {
        *outError = strdup("Not a JSON object: pack section");
        return NULL;
    }

    LIBMATTI_MC_PackMetadataSection *section = calloc(1, sizeof(LIBMATTI_MC_PackMetadataSection));
    LIBMATTI_GSON_JsonElement *description = LIBMATTI_GSON_JsonElement_GetMember(object, "description");
    section->description = description != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(description) : strdup("");

    // Java: "pack_format" (int)
    LIBMATTI_GSON_JsonElement *format = LIBMATTI_GSON_JsonElement_GetMember(object, "pack_format");
    if (format != NULL)
    {
        section->descriptionFormat.hasFormat = 1;
        section->descriptionFormat.format = LIBMATTI_GSON_JsonElement_GetAsInt(format);
    }

    // Java: "supported_formats" / "pack_format_border" (inclusive range, int or [min, max])
    LIBMATTI_GSON_JsonElement *supported = LIBMATTI_GSON_JsonElement_GetMember(object, "supported_formats");
    if (supported == NULL) supported = LIBMATTI_GSON_JsonElement_GetMember(object, "pack_format_border");
    if (supported != NULL)
    {
        int min;
        int max;
        if (LIBMATTI_GSON_JsonElement_IsJsonArray(supported))
        {
            if (LIBMATTI_GSON_JsonElement_ElementCount(supported) >= 2)
            {
                min = LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(supported, 0));
                max = LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(supported, 1));
            }
            else
            {
                *outError = strdup("supported_formats array needs two entries");
                LIBMATTI_MC_PackMetadataSection_Free(section);
                return NULL;
            }
        }
        else
        {
            min = LIBMATTI_GSON_JsonElement_GetAsInt(supported);
            max = min;
        }
        section->descriptionFormat.hasSupported = 1;
        section->descriptionFormat.supported = LIBMATTI_MC_InclusiveRange_Of(min, max);
    }

    // Java: "min_format"/"max_format" (the IntermediaryFormat)
    LIBMATTI_GSON_JsonElement *minFormat = LIBMATTI_GSON_JsonElement_GetMember(object, "min_format");
    LIBMATTI_GSON_JsonElement *maxFormat = LIBMATTI_GSON_JsonElement_GetMember(object, "max_format");
    if (minFormat != NULL && maxFormat != NULL)
    {
        section->descriptionFormat.hasMin = 1;
        if (LIBMATTI_GSON_JsonElement_IsJsonArray(minFormat) && LIBMATTI_GSON_JsonElement_ElementCount(minFormat) >= 2)
            section->descriptionFormat.min =
                LIBMATTI_MC_PackFormat_Of(LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(minFormat, 0)),
                                          LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(minFormat, 1)));
        else
            section->descriptionFormat.min = LIBMATTI_MC_PackFormat_OfSingle(LIBMATTI_GSON_JsonElement_GetAsInt(minFormat));
        section->descriptionFormat.hasMax = 1;
        if (LIBMATTI_GSON_JsonElement_IsJsonArray(maxFormat) && LIBMATTI_GSON_JsonElement_ElementCount(maxFormat) >= 2)
            section->descriptionFormat.max =
                LIBMATTI_MC_PackFormat_Of(LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(maxFormat, 0)),
                                          LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(maxFormat, 1)));
        else
            section->descriptionFormat.max = LIBMATTI_MC_PackFormat_OfSingle(LIBMATTI_GSON_JsonElement_GetAsInt(maxFormat));
    }

    return section;
}

void LIBMATTI_MC_PackMetadataSection_Free(LIBMATTI_MC_PackMetadataSection *section)
{
    free(section->description);
    free(section);
}

// ---------------------------------------------------------------------------
// ResourceFilterSection
// ---------------------------------------------------------------------------

static void filter_pattern_free(LIBMATTI_MC_ResourceFilterSection_Pattern *pattern)
{
    free(pattern->namespace);
    free(pattern->path);
}

void LIBMATTI_MC_ResourceFilterSection_Free(LIBMATTI_MC_ResourceFilterSection *section)
{
    for (size_t i = 0; i < section->patternCount; i++)
        filter_pattern_free(&section->patterns[i]);
    free(section->patterns);
    free(section);
}

static int parse_pattern_list(const LIBMATTI_GSON_JsonElement *object, const char *member,
                              LIBMATTI_MC_ResourceFilterSection_Pattern **outPatterns, size_t *outCount,
                              char **outError)
{
    LIBMATTI_GSON_JsonElement *array = LIBMATTI_GSON_JsonElement_GetMember(object, member);
    if (array == NULL)
    {
        *outPatterns = NULL;
        *outCount = 0;
        return 1;
    }
    if (!LIBMATTI_GSON_JsonElement_IsJsonArray(array))
    {
        *outError = strdup("Filter patterns must be an array");
        return 0;
    }
    size_t count = LIBMATTI_GSON_JsonElement_ElementCount(array);
    LIBMATTI_MC_ResourceFilterSection_Pattern *patterns =
        calloc(count > 0 ? count : 1, sizeof(LIBMATTI_MC_ResourceFilterSection_Pattern));
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_GSON_JsonElement *entry = LIBMATTI_GSON_JsonElement_ElementAt(array, i);
        LIBMATTI_GSON_JsonElement *patternObject = LIBMATTI_GSON_JsonElement_GetAsJsonObject(entry);
        if (patternObject == NULL)
        {
            *outError = strdup("Filter pattern must be an object");
            free(patterns);
            return 0;
        }
        LIBMATTI_GSON_JsonElement *ns = LIBMATTI_GSON_JsonElement_GetMember(patternObject, "namespace");
        LIBMATTI_GSON_JsonElement *path = LIBMATTI_GSON_JsonElement_GetMember(patternObject, "path");
        patterns[i].namespace = ns != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(ns) : strdup("");
        patterns[i].path = path != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(path) : strdup("");
    }
    *outPatterns = patterns;
    *outCount = count;
    return 1;
}

LIBMATTI_MC_ResourceFilterSection *LIBMATTI_MC_ResourceFilterSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError)
{
    (void) self;
    *outError = NULL;
    LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_JsonElement_GetAsJsonObject((LIBMATTI_GSON_JsonElement *) element);
    if (object == NULL)
    {
        *outError = strdup("Not a JSON object: filter section");
        return NULL;
    }
    LIBMATTI_MC_ResourceFilterSection *section = calloc(1, sizeof(LIBMATTI_MC_ResourceFilterSection));
    if (!parse_pattern_list(object, "block", &section->patterns, &section->patternCount, outError))
    {
        LIBMATTI_MC_ResourceFilterSection_Free(section);
        return NULL;
    }
    return section;
}

// ---------------------------------------------------------------------------
// FeatureFlagsMetadataSection
// ---------------------------------------------------------------------------

void LIBMATTI_MC_FeatureFlagsMetadataSection_Free(LIBMATTI_MC_FeatureFlagsMetadataSection *section)
{
    for (size_t i = 0; i < section->flagCount; i++)
        free(section->flags[i]);
    free(section->flags);
    free(section);
}

LIBMATTI_MC_FeatureFlagsMetadataSection *LIBMATTI_MC_FeatureFlagsMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError)
{
    (void) self;
    *outError = NULL;
    LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_JsonElement_GetAsJsonObject((LIBMATTI_GSON_JsonElement *) element);
    if (object == NULL)
    {
        *outError = strdup("Not a JSON object: feature_flags section");
        return NULL;
    }
    LIBMATTI_MC_FeatureFlagsMetadataSection *section = calloc(1, sizeof(LIBMATTI_MC_FeatureFlagsMetadataSection));
    LIBMATTI_GSON_JsonElement *flags = LIBMATTI_GSON_JsonElement_GetMember(object, "enabled_flags");
    if (flags != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray(flags))
    {
        size_t count = LIBMATTI_GSON_JsonElement_ElementCount(flags);
        section->flags = calloc(count > 0 ? count : 1, sizeof(char *));
        for (size_t i = 0; i < count; i++)
            section->flags[i] = LIBMATTI_GSON_JsonElement_GetAsString(LIBMATTI_GSON_JsonElement_ElementAt(flags, i));
        section->flagCount = count;
    }
    return section;
}

// ---------------------------------------------------------------------------
// OverlayMetadataSection
// ---------------------------------------------------------------------------

void LIBMATTI_MC_OverlayMetadataSection_Free(LIBMATTI_MC_OverlayMetadataSection *section)
{
    for (size_t i = 0; i < section->entryCount; i++)
    {
        free(section->directories[i]);
    }
    free(section->directories);
    free(section->formats);
    free(section);
}

LIBMATTI_MC_OverlayMetadataSection *LIBMATTI_MC_OverlayMetadataSection_FromJson(
    const LIBMATTI_MC_MetadataSectionSerializer *self, const LIBMATTI_GSON_JsonElement *element, char **outError)
{
    (void) self;
    *outError = NULL;
    LIBMATTI_GSON_JsonElement *object = LIBMATTI_GSON_JsonElement_GetAsJsonObject((LIBMATTI_GSON_JsonElement *) element);
    if (object == NULL)
    {
        *outError = strdup("Not a JSON object: overlays section");
        return NULL;
    }
    LIBMATTI_MC_OverlayMetadataSection *section = calloc(1, sizeof(LIBMATTI_MC_OverlayMetadataSection));
    LIBMATTI_GSON_JsonElement *overlays = LIBMATTI_GSON_JsonElement_GetMember(object, "entries");
    if (overlays == NULL) overlays = LIBMATTI_GSON_JsonElement_GetMember(object, "overlays");
    if (overlays != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray(overlays))
    {
        size_t count = LIBMATTI_GSON_JsonElement_ElementCount(overlays);
        section->directories = calloc(count > 0 ? count : 1, sizeof(char *));
        section->formats = calloc(count > 0 ? count : 1, sizeof(LIBMATTI_MC_PackFormat_IntermediaryFormat));
        for (size_t i = 0; i < count; i++)
        {
            LIBMATTI_GSON_JsonElement *entryObject =
                LIBMATTI_GSON_JsonElement_GetAsJsonObject(LIBMATTI_GSON_JsonElement_ElementAt(overlays, i));
            if (entryObject == NULL) continue;
            LIBMATTI_GSON_JsonElement *directory = LIBMATTI_GSON_JsonElement_GetMember(entryObject, "directory");
            section->directories[i] =
                directory != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(directory) : strdup("");
            LIBMATTI_MC_PackFormat_IntermediaryFormat holder;
            memset(&holder, 0, sizeof(holder));
            LIBMATTI_GSON_JsonElement *formats = LIBMATTI_GSON_JsonElement_GetMember(entryObject, "formats");
            if (formats != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray(formats) &&
                LIBMATTI_GSON_JsonElement_ElementCount(formats) >= 2)
            {
                holder.hasSupported = 1;
                holder.supported = LIBMATTI_MC_InclusiveRange_Of(
                    LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(formats, 0)),
                    LIBMATTI_GSON_JsonElement_GetAsInt(LIBMATTI_GSON_JsonElement_ElementAt(formats, 1)));
            }
            section->formats[i] = holder;
        }
        section->entryCount = count;
    }
    return section;
}
