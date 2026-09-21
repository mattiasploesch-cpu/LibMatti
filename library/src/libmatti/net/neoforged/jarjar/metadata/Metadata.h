// Port of net.neoforged.jarjar.metadata (ContainedJarIdentifier,
// ContainedJarMetadata, ContainedVersion, Metadata).
// Jar-in-jar at .so level: the manifest section ".jarjar" lists the contained
// artifacts the loader resolves from its own library set.

#ifndef MATTICRAFT_JARJAR_METADATA_H
#define MATTICRAFT_JARJAR_METADATA_H

#include <stddef.h>

// Java: record ContainedJarIdentifier(String group, String artifact)
typedef struct
{
    char *group;
    char *artifact;
} LIBMATTI_JARJAR_ContainedJarIdentifier;

// Java: record ContainedVersion(String range) - the maven version range string
typedef struct
{
    char *range;
} LIBMATTI_JARJAR_ContainedVersion;

// Java: record ContainedJarMetadata(ContainedJarIdentifier identifier, ContainedVersion version)
typedef struct
{
    LIBMATTI_JARJAR_ContainedJarIdentifier identifier;
    LIBMATTI_JARJAR_ContainedVersion version;
} LIBMATTI_JARJAR_ContainedJarMetadata;

// Java: record Metadata(List<ContainedJarMetadata> jars)
typedef struct
{
    LIBMATTI_JARJAR_ContainedJarMetadata *jars;
    size_t jarCount;
} LIBMATTI_JARJAR_Metadata;

LIBMATTI_JARJAR_Metadata *LIBMATTI_JARJAR_Metadata_New(void);
void LIBMATTI_JARJAR_Metadata_Free(LIBMATTI_JARJAR_Metadata *metadata);

// Java: MetadataIOHandler.fromJSONText(String) - parses the "jarjar.metadata"
// JSON: {"jars":[{"identifier":{"group":"g","artifact":"a"},"version":{"range":"[1.0,2.0)"}}]}
LIBMATTI_JARJAR_Metadata *LIBMATTI_JARJAR_MetadataIOHandler_FromJSONText(const char *jsonText);
// Java: toJSONText()
char *LIBMATTI_JARJAR_MetadataIOHandler_ToJSONText(const LIBMATTI_JARJAR_Metadata *metadata);

#endif //MATTICRAFT_JARJAR_METADATA_H
