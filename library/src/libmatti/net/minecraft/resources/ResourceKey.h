// Port of net.minecraft.resources.ResourceKey.
// Java interns every key through a ConcurrentMap keyed by (registry, identifier); the C port
// keeps the same intern table so two create() calls for the same pair return one object and
// pointer equality works like Java's identity semantics.

#ifndef MATTICRAFT_NET_MINECRAFT_RESOURCES_RESOURCEKEY_H
#define MATTICRAFT_NET_MINECRAFT_RESOURCES_RESOURCEKEY_H

#include "libmatti/net/minecraft/resources/Identifier.h"

// Java: public class ResourceKey<T>
typedef struct LIBMATTI_MC_ResourceKey LIBMATTI_MC_ResourceKey;

struct LIBMATTI_MC_ResourceKey
{
    // Java: private final Identifier registryName;
    LIBMATTI_MC_Identifier *registryName;
    // Java: private final Identifier identifier;
    LIBMATTI_MC_Identifier *identifier;
};

// Java: private static <T> ResourceKey<T> create(Identifier registry, Identifier identifier) -
// interned; the returned key is owned by the intern table (never freed by the caller)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateInterned(const LIBMATTI_MC_Identifier *registry,
                                                                const LIBMATTI_MC_Identifier *identifier);
// Java: public static <T> ResourceKey<T> create(ResourceKey<? extends Registry<T>> key, Identifier identifier)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_Create(const LIBMATTI_MC_ResourceKey *registryKey,
                                                        const LIBMATTI_MC_Identifier *identifier);
// Java: public static <T> ResourceKey<Registry<T>> createRegistryKey(Identifier identifier) -
// registry name is Registries.ROOT_REGISTRY_NAME ("minecraft:root")
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateRegistryKey(const LIBMATTI_MC_Identifier *identifier);
// convenience: createRegistryKey from a plain "namespace:path" string
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_CreateRegistryKeyParsed(const char *location);

// Java: public Identifier identifier() / registry()
const LIBMATTI_MC_Identifier *LIBMATTI_MC_ResourceKey_Identifier(const LIBMATTI_MC_ResourceKey *key);
const LIBMATTI_MC_Identifier *LIBMATTI_MC_ResourceKey_Registry(const LIBMATTI_MC_ResourceKey *key);

// Java: public ResourceKey<Registry<T>> registryKey()
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_ResourceKey_RegistryKey(const LIBMATTI_MC_ResourceKey *key);

// Java: public boolean isFor(ResourceKey<? extends Registry<?>> registryTracker) -
// registryName.equals(registryTracker.identifier())
int LIBMATTI_MC_ResourceKey_IsFor(const LIBMATTI_MC_ResourceKey *key, const LIBMATTI_MC_ResourceKey *registryTracker);

// Java: public String toString() - "ResourceKey[registry / identifier]"; caller frees
char *LIBMATTI_MC_ResourceKey_ToString(const LIBMATTI_MC_ResourceKey *key);

// Java: the intern table returns shared instances - two keys with the same pair are the same
// object (pointer equality), exactly Java's identity semantics
int LIBMATTI_MC_ResourceKey_Same(const LIBMATTI_MC_ResourceKey *a, const LIBMATTI_MC_ResourceKey *b);

#endif //MATTICRAFT_NET_MINECRAFT_RESOURCES_RESOURCEKEY_H
