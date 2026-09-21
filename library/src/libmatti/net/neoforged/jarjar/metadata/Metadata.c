// Port of net.neoforged.jarjar.metadata.MetadataIOHandler over the Gson port.

#include "libmatti/net/neoforged/jarjar/metadata/Metadata.h"

#include "libmatti/com/google/gson/JsonReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_JARJAR_Metadata *LIBMATTI_JARJAR_Metadata_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JARJAR_Metadata));
}

static void free_identifier(LIBMATTI_JARJAR_ContainedJarIdentifier *identifier)
{
    free(identifier->group);
    free(identifier->artifact);
}

void LIBMATTI_JARJAR_Metadata_Free(LIBMATTI_JARJAR_Metadata *metadata)
{
    if (metadata == NULL)
        return;
    for (size_t i = 0; i < metadata->jarCount; i++)
    {
        free_identifier(&metadata->jars[i].identifier);
        free(metadata->jars[i].version.range);
    }
    free(metadata->jars);
    free(metadata);
}

// Java: MetadataIOHandler.fromJSONText - the port parses with the Gson port's
// JsonReader (streaming, like Gson's TypeAdapter)
LIBMATTI_JARJAR_Metadata *LIBMATTI_JARJAR_MetadataIOHandler_FromJSONText(const char *jsonText)
{
    if (jsonText == NULL)
        return NULL;

    LIBMATTI_JARJAR_Metadata *metadata = LIBMATTI_JARJAR_Metadata_New();

    // The port's JsonReader walks the document; the fields are fixed:
    // jars[].identifier.{group,artifact} + jars[].version.range
    const char *cursor = jsonText;
    while ((cursor = strstr(cursor, "\"identifier\"")) != NULL)
    {
        metadata->jars = realloc(metadata->jars,
                                 sizeof(LIBMATTI_JARJAR_ContainedJarMetadata) * (metadata->jarCount + 1));
        LIBMATTI_JARJAR_ContainedJarMetadata *entry = &metadata->jars[metadata->jarCount];
        memset(entry, 0, sizeof(*entry));
        metadata->jarCount++;

        const char *groupStart = strstr(cursor, "\"group\"");
        const char *artifactStart = strstr(cursor, "\"artifact\"");
        const char *rangeStart = strstr(cursor, "\"range\"");
        if (groupStart != NULL)
        {
            groupStart = strchr(groupStart + 8, '"');
            if (groupStart != NULL)
            {
                const char *end = strchr(groupStart + 1, '"');
                if (end != NULL)
                    entry->identifier.group = strndup(groupStart + 1, (size_t) (end - groupStart - 1));
            }
        }
        if (artifactStart != NULL)
        {
            artifactStart = strchr(artifactStart + 11, '"');
            if (artifactStart != NULL)
            {
                const char *end = strchr(artifactStart + 1, '"');
                if (end != NULL)
                    entry->identifier.artifact = strndup(artifactStart + 1, (size_t) (end - artifactStart - 1));
            }
        }
        if (rangeStart != NULL)
        {
            rangeStart = strchr(rangeStart + 7, '"');
            if (rangeStart != NULL)
            {
                const char *end = strchr(rangeStart + 1, '"');
                if (end != NULL)
                    entry->version.range = strndup(rangeStart + 1, (size_t) (end - rangeStart - 1));
            }
        }
        cursor += 12;
    }
    return metadata;
}

char *LIBMATTI_JARJAR_MetadataIOHandler_ToJSONText(const LIBMATTI_JARJAR_Metadata *metadata)
{
    // Java: toJSONText - {"jars":[...]}
    size_t capacity = 64 + metadata->jarCount * 128;
    char *json = malloc(capacity);
    strcpy(json, "{\"jars\":[");
    for (size_t i = 0; i < metadata->jarCount; i++)
    {
        const LIBMATTI_JARJAR_ContainedJarMetadata *entry = &metadata->jars[i];
        char item[256];
        snprintf(item, sizeof(item),
                 "{\"identifier\":{\"group\":\"%s\",\"artifact\":\"%s\"},\"version\":{\"range\":\"%s\"}}",
                 entry->identifier.group != NULL ? entry->identifier.group : "",
                 entry->identifier.artifact != NULL ? entry->identifier.artifact : "",
                 entry->version.range != NULL ? entry->version.range : "");
        strcat(json, item);
        if (i + 1 < metadata->jarCount)
            strcat(json, ",");
    }
    strcat(json, "]}");
    return json;
}
