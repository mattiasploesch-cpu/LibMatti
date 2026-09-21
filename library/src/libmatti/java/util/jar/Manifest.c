//
// Created by administrator on 09.09.26.
//

#include "Manifest.h"

#include <stdlib.h>
#include <string.h>

static void attributes_free(LIBMATTI_JU_Attributes *attributes)
{
    for (size_t i = 0; i < attributes->count; i++)
    {
        free(attributes->keys[i]);
        free(attributes->values[i]);
    }
    free(attributes->keys);
    free(attributes->values);
    attributes->keys = NULL;
    attributes->values = NULL;
    attributes->count = 0;
}

static void attributes_set(LIBMATTI_JU_Attributes *attributes, const char *key, const char *value)
{
    for (size_t i = 0; i < attributes->count; i++)
    {
        if (strcmp(attributes->keys[i], key) == 0)
        {
            free(attributes->values[i]);
            attributes->values[i] = strdup(value);
            return;
        }
    }

    attributes->keys = realloc(attributes->keys, sizeof(char *) * (attributes->count + 1));
    attributes->values = realloc(attributes->values, sizeof(char *) * (attributes->count + 1));
    attributes->keys[attributes->count] = strdup(key);
    attributes->values[attributes->count] = strdup(value);
    attributes->count++;
}

const char *LIBMATTI_JU_Attributes_GetValue(const LIBMATTI_JU_Attributes *attributes, const char *key)
{
    for (size_t i = 0; i < attributes->count; i++)
    {
        if (strcmp(attributes->keys[i], key) == 0) return attributes->values[i];
    }
    return NULL;
}

LIBMATTI_JU_Manifest *LIBMATTI_JU_Manifest_CreateEmpty(void)
{
    return calloc(1, sizeof(LIBMATTI_JU_Manifest));
}

void LIBMATTI_JU_Manifest_Free(LIBMATTI_JU_Manifest *manifest)
{
    if (manifest == NULL) return;

    attributes_free(&manifest->mainAttributes);

    for (size_t i = 0; i < manifest->sectionCount; i++)
    {
        free(manifest->sections[i].name);
        attributes_free(&manifest->sections[i].attributes);
    }
    free(manifest->sections);
    free(manifest);
}

const char *LIBMATTI_JU_Manifest_GetMainValue(const LIBMATTI_JU_Manifest *manifest, const char *key)
{
    return LIBMATTI_JU_Attributes_GetValue(&manifest->mainAttributes, key);
}

const LIBMATTI_JU_Attributes *LIBMATTI_JU_Manifest_GetSection(const LIBMATTI_JU_Manifest *manifest, const char *name)
{
    for (size_t i = 0; i < manifest->sectionCount; i++)
    {
        if (strcmp(manifest->sections[i].name, name) == 0) return &manifest->sections[i].attributes;
    }
    return NULL;
}

LIBMATTI_JU_Manifest *LIBMATTI_JU_Manifest_Parse(const char *bytes, size_t length)
{
    LIBMATTI_JU_Manifest *manifest = calloc(1, sizeof(LIBMATTI_JU_Manifest));
    if (manifest == NULL) return NULL;

    // Split into lines (handle \r\n, \n and \r)
    char *copy = malloc(length + 1);
    memcpy(copy, bytes, length);
    copy[length] = '\0';

    char **lines = NULL;
    size_t lineCount = 0;
    char *saveptr;
    char *line = strtok_r(copy, "\n", &saveptr);

    while (line != NULL)
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';

        lines = realloc(lines, sizeof(char *) * (lineCount + 1));
        lines[lineCount] = strdup(line);
        lineCount++;
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(copy);

    // Java: java.util.jar.Attributes.read - "Name: " starts a new section,
    // lines starting with a space are continuations of the previous value
    LIBMATTI_JU_Attributes *current = &manifest->mainAttributes;
    int inSection = 0;

    for (size_t i = 0; i < lineCount; i++)
    {
        const char *l = lines[i];

        if (l[0] == ' ')
        {
            // Continuation line: append to the previous value
            const char *content = l + 1;
            if (current->count > 0)
            {
                size_t oldLen = strlen(current->values[current->count - 1]);
                size_t addLen = strlen(content);
                char *joined = malloc(oldLen + addLen + 1);
                memcpy(joined, current->values[current->count - 1], oldLen);
                memcpy(joined + oldLen, content, addLen);
                joined[oldLen + addLen] = '\0';
                free(current->values[current->count - 1]);
                current->values[current->count - 1] = joined;
            }
            continue;
        }

        if (l[0] == '\0')
        {
            // Blank line: end of the current section, next section starts after
            inSection = 0;
            continue;
        }

        const char *colon = strchr(l, ':');
        if (colon == NULL) continue;

        size_t keyLen = (size_t)(colon - l);
        const char *value = colon + 1;
        if (value[0] == ' ') value++; // strip the single space after the colon

        char *key = malloc(keyLen + 1);
        memcpy(key, l, keyLen);
        key[keyLen] = '\0';

        // Java: !in && line.startsWith("Name: ") starts a new section
        if (!inSection && strcmp(key, "Name") == 0)
        {
            manifest->sections = realloc(manifest->sections, sizeof(LIBMATTI_JU_ManifestSection) * (manifest->sectionCount + 1));
            LIBMATTI_JU_ManifestSection *section = &manifest->sections[manifest->sectionCount];
            section->name = strdup(value);
            memset(&section->attributes, 0, sizeof(LIBMATTI_JU_Attributes));
            manifest->sectionCount++;
            current = &section->attributes;
            inSection = 1;
        }
        else
        {
            attributes_set(current, key, value);
            inSection = 1;
        }

        free(key);
    }

    for (size_t i = 0; i < lineCount; i++) free(lines[i]);
    free(lines);

    return manifest;
}