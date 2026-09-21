// Port of net.minecraft.resources.ResourceKey.

#include "libmatti/net/minecraft/resources/ResourceKey.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final ConcurrentMap<ResourceKey.InternKey, ResourceKey<?>> VALUES
// - the C port keeps a growable array of interned keys; entries live for the process lifetime
// like Java's weak-values map does for the reachable keys.
typedef struct
{
    LIBMATTI_MC_Identifier *registry;
    LIBMATTI_MC_Identifier *identifier;
    LIBMATTI_MC_ResourceKey *key;
} InternEntry;

static InternEntry *internTable = NULL;
static size_t internCount = 0;
static size_t internCapacity = 0;

static LIBMATTI_MC_ResourceKey *intern(const LIBMATTI_MC_Identifier *registry,
                                       const LIBMATTI_MC_Identifier *identifier)
{
    for (size_t i = 0; i < internCount; i++)
    {
        if (LIBMATTI_MC_Identifier_Equals(internTable[i].registry, registry) &&
            LIBMATTI_MC_Identifier_Equals(internTable[i].identifier, identifier))
            return internTable[i].key;
    }

    if (internCount == internCapacity)
    {
        internCapacity = internCapacity > 0 ? internCapacity * 2 : 16;
        internTable = realloc(internTable, sizeof(InternEntry) * internCapacity);
    }

    LIBMATTI_MC_ResourceKey *key = calloc(1, sizeof(LIBMATTI_MC_ResourceKey));
    // Java: the identifiers are shared references from the caller; the intern table owns its
    // own copies so the caller's lifecycle does not matter
    key->registryName = LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_GetNamespace(registry),
                                                   LIBMATTI_MC_Identifier_GetPath(registry));
    key->identifier = LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_GetNamespace(identifier),
                                                 LIBMATTI_MC_Identifier_GetPath(identifier));
    internTable[internCount].registry = key->registryName;
    internTable[internCount].identifier = key->identifier;
    internTable[internCount].key = key;
    internCount++;
    return key;
}

// Java: private static <T> ResourceKey<T> create(Identifier registry, Identifier identifier)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateInterned(const LIBMATTI_MC_Identifier *registry,
                                                                const LIBMATTI_MC_Identifier *identifier)
{
    return intern(registry, identifier);
}

// Java: public static <T> ResourceKey<T> create(ResourceKey<? extends Registry<T>> key, Identifier identifier)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_Create(const LIBMATTI_MC_ResourceKey *registryKey,
                                                        const LIBMATTI_MC_Identifier *identifier)
{
    return intern(registryKey->identifier, identifier);
}

// Java: public static <T> ResourceKey<Registry<T>> createRegistryKey(Identifier identifier) -
// create(Registries.ROOT_REGISTRY_NAME, identifier)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateRegistryKey(const LIBMATTI_MC_Identifier *identifier)
{
    static LIBMATTI_MC_Identifier *rootRegistryName = NULL;
    if (rootRegistryName == NULL)
        rootRegistryName = LIBMATTI_MC_Identifier_Parse("minecraft:root");
    return intern(rootRegistryName, identifier);
}

LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateRegistryKeyParsed(const char *location)
{
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(location);
    LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_ResourceKey_CreateRegistryKey(identifier);
    LIBMATTI_MC_Identifier_Free(identifier);
    return key;
}

// Java: public Identifier identifier()
const LIBMATTI_MC_Identifier *LIBMATTI_MC_ResourceKey_Identifier(const LIBMATTI_MC_ResourceKey *key)
{
    return key->identifier;
}

// Java: public Identifier registry()
const LIBMATTI_MC_Identifier *LIBMATTI_MC_ResourceKey_Registry(const LIBMATTI_MC_ResourceKey *key)
{
    return key->registryName;
}

// Java: public ResourceKey<Registry<T>> registryKey() - createRegistryKey(registryName)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_RegistryKey(const LIBMATTI_MC_ResourceKey *key)
{
    return LIBMATTI_MC_ResourceKey_CreateRegistryKey(key->registryName);
}

// Java: public boolean isFor(ResourceKey<? extends Registry<?>> registryTracker)
int LIBMATTI_MC_ResourceKey_IsFor(const LIBMATTI_MC_ResourceKey *key, const LIBMATTI_MC_ResourceKey *registryTracker)
{
    return LIBMATTI_MC_Identifier_Equals(key->registryName, registryTracker->identifier);
}

// Java: public String toString() - "ResourceKey[" + registryName + " / " + identifier + "]"
char *LIBMATTI_MC_ResourceKey_ToString(const LIBMATTI_MC_ResourceKey *key)
{
    char *registry = LIBMATTI_MC_Identifier_ToString(key->registryName);
    char *identifier = LIBMATTI_MC_Identifier_ToString(key->identifier);
    size_t size = strlen("ResourceKey[]") + strlen(registry) + strlen(" / ") + strlen(identifier) + 1;
    char *string = malloc(size);
    snprintf(string, size, "ResourceKey[%s / %s]", registry, identifier);
    free(registry);
    free(identifier);
    return string;
}

// Java: the interned keys are identity-compared
int LIBMATTI_MC_ResourceKey_Same(const LIBMATTI_MC_ResourceKey *a, const LIBMATTI_MC_ResourceKey *b)
{
    return a == b;
}
