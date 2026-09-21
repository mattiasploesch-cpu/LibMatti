//
// Created by administrator on 09.09.26.
//
// Port of java.util.jar.Manifest.

#ifndef MATTICRAFT_MANIFEST_H
#define MATTICRAFT_MANIFEST_H

#include <stddef.h>

typedef struct
{
    char **keys;
    char **values;
    size_t count;
} LIBMATTI_JU_Attributes;

typedef struct
{
    char *name; // section name (entry path)
    LIBMATTI_JU_Attributes attributes;
} LIBMATTI_JU_ManifestSection;

typedef struct
{
    LIBMATTI_JU_Attributes mainAttributes; // Java: getMainAttributes()
    LIBMATTI_JU_ManifestSection *sections;
    size_t sectionCount;
} LIBMATTI_JU_Manifest;

// Java: new Manifest()
LIBMATTI_JU_Manifest *LIBMATTI_JU_Manifest_CreateEmpty(void);
// Java: new Manifest(InputStream); NULL on parse failure
LIBMATTI_JU_Manifest *LIBMATTI_JU_Manifest_Parse(const char *bytes, size_t length);
void LIBMATTI_JU_Manifest_Free(LIBMATTI_JU_Manifest *manifest);

// Java: mainAttributes.getValue(Name)
const char *LIBMATTI_JU_Manifest_GetMainValue(const LIBMATTI_JU_Manifest *manifest, const char *key);
// Java: getAttributes(String entryName); NULL if the section does not exist
const LIBMATTI_JU_Attributes *LIBMATTI_JU_Manifest_GetSection(const LIBMATTI_JU_Manifest *manifest, const char *name);
const char *LIBMATTI_JU_Attributes_GetValue(const LIBMATTI_JU_Attributes *attributes, const char *key);

#endif //MATTICRAFT_MANIFEST_H