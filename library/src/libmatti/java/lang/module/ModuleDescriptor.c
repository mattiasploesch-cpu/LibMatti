//
// Created by administrator on 09.09.26.
//

#include "ModuleDescriptor.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Version
// ---------------------------------------------------------------------------

LIBMATTI_JL_ModuleDescriptor_Version *LIBMATTI_JL_ModuleDescriptor_Version_Parse(const char *version)
{
    LIBMATTI_JL_ModuleDescriptor_Version *result = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor_Version));
    result->raw = strdup(version);

    // Java: Version.parse - components separated by '.', '-' or '+'
    char *copy = strdup(version);
    char *token = strtok(copy, ".-+");

    while (token != NULL)
    {
        result->parts = realloc(result->parts, sizeof(char *) * (result->partCount + 1));
        result->parts[result->partCount] = strdup(token);
        result->partCount++;
        token = strtok(NULL, ".-+");
    }

    free(copy);
    return result;
}

void LIBMATTI_JL_ModuleDescriptor_Version_Free(LIBMATTI_JL_ModuleDescriptor_Version *version)
{
    if (version == NULL) return;

    free(version->raw);
    for (size_t i = 0; i < version->partCount; i++) free(version->parts[i]);
    free(version->parts);
    free(version);
}

static int version_part_is_numeric(const char *part)
{
    for (const char *c = part; *c != '\0'; c++)
    {
        if (!isdigit((unsigned char)*c)) return 0;
    }
    return 1;
}

int LIBMATTI_JL_ModuleDescriptor_Version_Compare(const LIBMATTI_JL_ModuleDescriptor_Version *a, const LIBMATTI_JL_ModuleDescriptor_Version *b)
{
    size_t common = a->partCount < b->partCount ? a->partCount : b->partCount;

    for (size_t i = 0; i < common; i++)
    {
        int aNumeric = version_part_is_numeric(a->parts[i]);
        int bNumeric = version_part_is_numeric(b->parts[i]);

        if (aNumeric && bNumeric)
        {
            long av = atol(a->parts[i]);
            long bv = atol(b->parts[i]);
            if (av != bv) return av < bv ? -1 : 1;
        }
        else if (aNumeric != bNumeric)
        {
            // Java: numeric components are greater than string components
            return aNumeric ? 1 : -1;
        }
        else
        {
            int cmp = strcmp(a->parts[i], b->parts[i]);
            if (cmp != 0) return cmp < 0 ? -1 : 1;
        }
    }

    if (a->partCount == b->partCount) return 0;
    return a->partCount < b->partCount ? -1 : 1;
}

const char *LIBMATTI_JL_ModuleDescriptor_Version_ToString(const LIBMATTI_JL_ModuleDescriptor_Version *version)
{
    return version->raw;
}

// ---------------------------------------------------------------------------
// ModuleDescriptor
// ---------------------------------------------------------------------------

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_Create(const char *name, char **packages, size_t packageCount)
{
    LIBMATTI_JL_ModuleDescriptor *descriptor = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor));

    descriptor->name = strdup(name);

    for (size_t i = 0; i < packageCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddPackage(descriptor, packages[i]);

    return descriptor;
}

void LIBMATTI_JL_ModuleDescriptor_Free(LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    if (descriptor == NULL) return;

    free(descriptor->name);
    free(descriptor->mainClass);
    LIBMATTI_JL_ModuleDescriptor_Version_Free(descriptor->version);

    for (size_t i = 0; i < descriptor->packageCount; i++) free(descriptor->packages[i]);
    free(descriptor->packages);

    for (size_t i = 0; i < descriptor->requiresCount; i++)
    {
        free(descriptor->requires[i]->name);
        LIBMATTI_JL_ModuleDescriptor_Version_Free(descriptor->requires[i]->version);
        free(descriptor->requires[i]);
    }
    free(descriptor->requires);

    for (size_t i = 0; i < descriptor->exportsCount; i++)
    {
        free(descriptor->exports[i]->source);
        for (size_t j = 0; j < descriptor->exports[i]->targetCount; j++) free(descriptor->exports[i]->targets[j]);
        free(descriptor->exports[i]->targets);
        free(descriptor->exports[i]);
    }
    free(descriptor->exports);

    for (size_t i = 0; i < descriptor->opensCount; i++)
    {
        free(descriptor->opens[i]->source);
        for (size_t j = 0; j < descriptor->opens[i]->targetCount; j++) free(descriptor->opens[i]->targets[j]);
        free(descriptor->opens[i]->targets);
        free(descriptor->opens[i]);
    }
    free(descriptor->opens);

    for (size_t i = 0; i < descriptor->usesCount; i++) free(descriptor->uses[i]);
    free(descriptor->uses);

    for (size_t i = 0; i < descriptor->providesCount; i++)
    {
        free(descriptor->provides[i]->service);
        for (size_t j = 0; j < descriptor->provides[i]->providerCount; j++) free(descriptor->provides[i]->providers[j]);
        free(descriptor->provides[i]->providers);
        free(descriptor->provides[i]);
    }
    free(descriptor->provides);

    free(descriptor);
}

const char *LIBMATTI_JL_ModuleDescriptor_Name(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return descriptor->name;
}

const char *LIBMATTI_JL_ModuleDescriptor_MainClass(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return descriptor->mainClass;
}

const LIBMATTI_JL_ModuleDescriptor_Version *LIBMATTI_JL_ModuleDescriptor_GetVersion(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return descriptor->version;
}

int LIBMATTI_JL_ModuleDescriptor_Modifiers(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return descriptor->modifiers;
}

int LIBMATTI_JL_ModuleDescriptor_IsOpen(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return (descriptor->modifiers & LIBMATTI_JL_MODIFIER_OPEN) != 0;
}

int LIBMATTI_JL_ModuleDescriptor_IsAutomatic(const LIBMATTI_JL_ModuleDescriptor *descriptor)
{
    return (descriptor->modifiers & LIBMATTI_JL_MODIFIER_AUTOMATIC) != 0;
}

char **LIBMATTI_JL_ModuleDescriptor_Packages(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->packageCount;
    return descriptor->packages;
}

LIBMATTI_JL_ModuleDescriptor_Requires **LIBMATTI_JL_ModuleDescriptor_GetRequires(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->requiresCount;
    return descriptor->requires;
}

LIBMATTI_JL_ModuleDescriptor_Exports **LIBMATTI_JL_ModuleDescriptor_GetExports(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->exportsCount;
    return descriptor->exports;
}

LIBMATTI_JL_ModuleDescriptor_Opens **LIBMATTI_JL_ModuleDescriptor_GetOpens(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->opensCount;
    return descriptor->opens;
}

char **LIBMATTI_JL_ModuleDescriptor_Uses(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->usesCount;
    return descriptor->uses;
}

LIBMATTI_JL_ModuleDescriptor_Provides **LIBMATTI_JL_ModuleDescriptor_GetProvides(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count)
{
    *count = descriptor->providesCount;
    return descriptor->provides;
}

void LIBMATTI_JL_ModuleDescriptor_SetVersion(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *version)
{
    LIBMATTI_JL_ModuleDescriptor_Version_Free(descriptor->version);
    descriptor->version = LIBMATTI_JL_ModuleDescriptor_Version_Parse(version);
}

void LIBMATTI_JL_ModuleDescriptor_SetMainClass(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *mainClass)
{
    free(descriptor->mainClass);
    descriptor->mainClass = strdup(mainClass);
}

void LIBMATTI_JL_ModuleDescriptor_AddModifier(LIBMATTI_JL_ModuleDescriptor *descriptor, LIBMATTI_JL_Modifier modifier)
{
    descriptor->modifiers |= modifier;
}

void LIBMATTI_JL_ModuleDescriptor_AddPackage(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *package)
{
    descriptor->packages = realloc(descriptor->packages, sizeof(char *) * (descriptor->packageCount + 1));
    descriptor->packages[descriptor->packageCount] = strdup(package);
    descriptor->packageCount++;
}

void LIBMATTI_JL_ModuleDescriptor_AddRequires(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *name, int modifiers, const char *version)
{
    LIBMATTI_JL_ModuleDescriptor_Requires *requires = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor_Requires));

    requires->name = strdup(name);
    requires->modifiers = modifiers;
    if (version != NULL)
        requires->version = LIBMATTI_JL_ModuleDescriptor_Version_Parse(version);

    descriptor->requires = realloc(descriptor->requires, sizeof(LIBMATTI_JL_ModuleDescriptor_Requires *) * (descriptor->requiresCount + 1));
    descriptor->requires[descriptor->requiresCount] = requires;
    descriptor->requiresCount++;
}

void LIBMATTI_JL_ModuleDescriptor_AddExports(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *source, char **targets, size_t targetCount)
{
    LIBMATTI_JL_ModuleDescriptor_Exports *exports = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor_Exports));

    exports->source = strdup(source);
    for (size_t i = 0; i < targetCount; i++)
    {
        exports->targets = realloc(exports->targets, sizeof(char *) * (exports->targetCount + 1));
        exports->targets[exports->targetCount] = strdup(targets[i]);
        exports->targetCount++;
    }

    descriptor->exports = realloc(descriptor->exports, sizeof(LIBMATTI_JL_ModuleDescriptor_Exports *) * (descriptor->exportsCount + 1));
    descriptor->exports[descriptor->exportsCount] = exports;
    descriptor->exportsCount++;
}

void LIBMATTI_JL_ModuleDescriptor_AddOpens(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *source, char **targets, size_t targetCount)
{
    LIBMATTI_JL_ModuleDescriptor_Opens *opens = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor_Opens));

    opens->source = strdup(source);
    for (size_t i = 0; i < targetCount; i++)
    {
        opens->targets = realloc(opens->targets, sizeof(char *) * (opens->targetCount + 1));
        opens->targets[opens->targetCount] = strdup(targets[i]);
        opens->targetCount++;
    }

    descriptor->opens = realloc(descriptor->opens, sizeof(LIBMATTI_JL_ModuleDescriptor_Opens *) * (descriptor->opensCount + 1));
    descriptor->opens[descriptor->opensCount] = opens;
    descriptor->opensCount++;
}

void LIBMATTI_JL_ModuleDescriptor_AddUses(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *service)
{
    descriptor->uses = realloc(descriptor->uses, sizeof(char *) * (descriptor->usesCount + 1));
    descriptor->uses[descriptor->usesCount] = strdup(service);
    descriptor->usesCount++;
}

void LIBMATTI_JL_ModuleDescriptor_AddProvides(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *service, char **providers, size_t providerCount)
{
    LIBMATTI_JL_ModuleDescriptor_Provides *provides = calloc(1, sizeof(LIBMATTI_JL_ModuleDescriptor_Provides));

    provides->service = strdup(service);
    for (size_t i = 0; i < providerCount; i++)
    {
        provides->providers = realloc(provides->providers, sizeof(char *) * (provides->providerCount + 1));
        provides->providers[provides->providerCount] = strdup(providers[i]);
        provides->providerCount++;
    }

    descriptor->provides = realloc(descriptor->provides, sizeof(LIBMATTI_JL_ModuleDescriptor_Provides *) * (descriptor->providesCount + 1));
    descriptor->provides[descriptor->providesCount] = provides;
    descriptor->providesCount++;
}