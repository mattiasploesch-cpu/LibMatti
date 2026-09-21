//
// Created by administrator on 09.09.26.
//
// Port of java.lang.module.ModuleDescriptor.

#ifndef MATTICRAFT_MODULEDESCRIPTOR_H
#define MATTICRAFT_MODULEDESCRIPTOR_H

#include <stddef.h>

// Java: class ModuleDescriptor.Version implements Comparable<Version>
typedef struct
{
    char *raw; // Java: Version.toString()
    char **parts; // components split on '.', '-', '+'
    size_t partCount;
} LIBMATTI_JL_ModuleDescriptor_Version;

// Java: enum ModuleDescriptor.Modifier { OPEN, AUTOMATIC, SYNTHETIC, MANDATED }
typedef enum
{
    LIBMATTI_JL_MODIFIER_OPEN = 1 << 0,
    LIBMATTI_JL_MODIFIER_AUTOMATIC = 1 << 1,
    LIBMATTI_JL_MODIFIER_SYNTHETIC = 1 << 2,
    LIBMATTI_JL_MODIFIER_MANDATED = 1 << 3
} LIBMATTI_JL_Modifier;

// Java: enum Requires.Modifier { TRANSITIVE, STATIC_PHASE, SYNTHETIC, MANDATED }
typedef enum
{
    LIBMATTI_JL_REQUIRES_MODIFIER_TRANSITIVE = 1 << 0,
    LIBMATTI_JL_REQUIRES_MODIFIER_STATIC_PHASE = 1 << 1,
    LIBMATTI_JL_REQUIRES_MODIFIER_SYNTHETIC = 1 << 2,
    LIBMATTI_JL_REQUIRES_MODIFIER_MANDATED = 1 << 3
} LIBMATTI_JL_RequiresModifier;

// Java: class ModuleDescriptor.Requires
typedef struct
{
    char *name;
    int modifiers; // LIBMATTI_JL_RequiresModifier bitmask
    LIBMATTI_JL_ModuleDescriptor_Version *version; // Optional<Version> (NULL = empty)
} LIBMATTI_JL_ModuleDescriptor_Requires;

// Java: class ModuleDescriptor.Exports
typedef struct
{
    char *source;
    char **targets; // Set<String> (NULL/0 = unqualified)
    size_t targetCount;
} LIBMATTI_JL_ModuleDescriptor_Exports;

// Java: class ModuleDescriptor.Opens
typedef struct
{
    char *source;
    char **targets; // Set<String> (NULL/0 = unqualified)
    size_t targetCount;
} LIBMATTI_JL_ModuleDescriptor_Opens;

// Java: class ModuleDescriptor.Provides
typedef struct
{
    char *service;
    char **providers; // List<String>
    size_t providerCount;
} LIBMATTI_JL_ModuleDescriptor_Provides;

// Java: class ModuleDescriptor
typedef struct
{
    char *name;
    char *mainClass; // Optional<String> (NULL = empty)
    LIBMATTI_JL_ModuleDescriptor_Version *version; // Optional<Version> (NULL = empty)
    int modifiers; // LIBMATTI_JL_Modifier bitmask
    char **packages; // Set<String>
    size_t packageCount;
    LIBMATTI_JL_ModuleDescriptor_Requires **requires; // Set<Requires>
    size_t requiresCount;
    LIBMATTI_JL_ModuleDescriptor_Exports **exports; // Set<Exports>
    size_t exportsCount;
    LIBMATTI_JL_ModuleDescriptor_Opens **opens; // Set<Opens>
    size_t opensCount;
    char **uses; // Set<String> (service types)
    size_t usesCount;
    LIBMATTI_JL_ModuleDescriptor_Provides **provides; // Set<Provides>
    size_t providesCount;
} LIBMATTI_JL_ModuleDescriptor;

// Java: Version.parse(String)
LIBMATTI_JL_ModuleDescriptor_Version *LIBMATTI_JL_ModuleDescriptor_Version_Parse(const char *version);
void LIBMATTI_JL_ModuleDescriptor_Version_Free(LIBMATTI_JL_ModuleDescriptor_Version *version);
// Java: Version.compareTo(Version) -> -1, 0, 1
int LIBMATTI_JL_ModuleDescriptor_Version_Compare(const LIBMATTI_JL_ModuleDescriptor_Version *a, const LIBMATTI_JL_ModuleDescriptor_Version *b);
const char *LIBMATTI_JL_ModuleDescriptor_Version_ToString(const LIBMATTI_JL_ModuleDescriptor_Version *version);

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JL_ModuleDescriptor_Create(const char *name, char **packages, size_t packageCount);
void LIBMATTI_JL_ModuleDescriptor_Free(LIBMATTI_JL_ModuleDescriptor *descriptor);

const char *LIBMATTI_JL_ModuleDescriptor_Name(const LIBMATTI_JL_ModuleDescriptor *descriptor);
const char *LIBMATTI_JL_ModuleDescriptor_MainClass(const LIBMATTI_JL_ModuleDescriptor *descriptor);
const LIBMATTI_JL_ModuleDescriptor_Version *LIBMATTI_JL_ModuleDescriptor_GetVersion(const LIBMATTI_JL_ModuleDescriptor *descriptor);
int LIBMATTI_JL_ModuleDescriptor_Modifiers(const LIBMATTI_JL_ModuleDescriptor *descriptor);
// Java: boolean isOpen() / isAutomatic()
int LIBMATTI_JL_ModuleDescriptor_IsOpen(const LIBMATTI_JL_ModuleDescriptor *descriptor);
int LIBMATTI_JL_ModuleDescriptor_IsAutomatic(const LIBMATTI_JL_ModuleDescriptor *descriptor);
// the following return the internal arrays, do not free
char **LIBMATTI_JL_ModuleDescriptor_Packages(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);
LIBMATTI_JL_ModuleDescriptor_Requires **LIBMATTI_JL_ModuleDescriptor_GetRequires(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);
LIBMATTI_JL_ModuleDescriptor_Exports **LIBMATTI_JL_ModuleDescriptor_GetExports(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);
LIBMATTI_JL_ModuleDescriptor_Opens **LIBMATTI_JL_ModuleDescriptor_GetOpens(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);
char **LIBMATTI_JL_ModuleDescriptor_Uses(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);
LIBMATTI_JL_ModuleDescriptor_Provides **LIBMATTI_JL_ModuleDescriptor_GetProvides(const LIBMATTI_JL_ModuleDescriptor *descriptor, size_t *count);

// Java: ModuleDescriptor.Builder equivalents
void LIBMATTI_JL_ModuleDescriptor_SetVersion(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *version);
void LIBMATTI_JL_ModuleDescriptor_SetMainClass(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *mainClass);
void LIBMATTI_JL_ModuleDescriptor_AddModifier(LIBMATTI_JL_ModuleDescriptor *descriptor, LIBMATTI_JL_Modifier modifier);
void LIBMATTI_JL_ModuleDescriptor_AddPackage(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *package);
void LIBMATTI_JL_ModuleDescriptor_AddRequires(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *name, int modifiers, const char *version);
void LIBMATTI_JL_ModuleDescriptor_AddExports(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *source, char **targets, size_t targetCount);
void LIBMATTI_JL_ModuleDescriptor_AddOpens(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *source, char **targets, size_t targetCount);
void LIBMATTI_JL_ModuleDescriptor_AddUses(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *service);
void LIBMATTI_JL_ModuleDescriptor_AddProvides(LIBMATTI_JL_ModuleDescriptor *descriptor, const char *service, char **providers, size_t providerCount);

#endif //MATTICRAFT_MODULEDESCRIPTOR_H