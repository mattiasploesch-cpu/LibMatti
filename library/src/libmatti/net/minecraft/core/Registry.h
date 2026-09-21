// Port of net.minecraft.core.Registry (the static register helpers and the interface surface
// the C port needs). The interface methods live on MappedRegistry; this header carries the
// Registry.register statics Java's Mods call and the Registry interface helpers that do not
// need the codec/holder stacks.

#ifndef MATTICRAFT_NET_MINECRAFT_CORE_REGISTRY_H
#define MATTICRAFT_NET_MINECRAFT_CORE_REGISTRY_H

#include "libmatti/net/minecraft/core/MappedRegistry.h"

// Java: static <T> T register(Registry<? super T> registry, String name, T value) -
// register(registry, Identifier.parse(name), value); returns value
void *LIBMATTI_MC_Registry_RegisterNamed(LIBMATTI_MC_MappedRegistry *registry, const char *name, void *value);

// Java: static <V, T extends V> T register(Registry<V> registry, Identifier id, T value) -
// register(registry, ResourceKey.create(registry.key(), id), value)
void *LIBMATTI_MC_Registry_RegisterWithIdentifier(LIBMATTI_MC_MappedRegistry *registry,
                                                  const LIBMATTI_MC_Identifier *identifier, void *value);

// Java: static <V, T extends V> T register(Registry<V> registry, ResourceKey<V> key, T value) -
// ((WritableRegistry) registry).register(key, value, RegistrationInfo.BUILT_IN); returns value
void *LIBMATTI_MC_Registry_Register(LIBMATTI_MC_MappedRegistry *registry,
                                    LIBMATTI_MC_ResourceKey *resourceKey, void *value);

// Java: the interface helpers without the codec part
// Java: default Optional<T> getOptional(@Nullable Identifier) - NULL when absent
void *LIBMATTI_MC_Registry_GetOptional(const LIBMATTI_MC_MappedRegistry *registry,
                                       const LIBMATTI_MC_Identifier *location);
// Java: default T getValueOrThrow(ResourceKey<T>) - aborts when the key is missing (Java throws)
void *LIBMATTI_MC_Registry_GetValueOrThrow(const LIBMATTI_MC_MappedRegistry *registry,
                                           const LIBMATTI_MC_ResourceKey *resourceKey);

#endif //MATTICRAFT_NET_MINECRAFT_CORE_REGISTRY_H
