// Port of net.minecraft.core.Registry (the static register helpers).

#include "libmatti/net/minecraft/core/Registry.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>

// Java: static <T> T register(Registry<? super T> registry, String name, T value)
void *LIBMATTI_MC_Registry_RegisterNamed(LIBMATTI_MC_MappedRegistry *registry, const char *name, void *value)
{
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(name);
    void *result = LIBMATTI_MC_Registry_RegisterWithIdentifier(registry, identifier, value);
    LIBMATTI_MC_Identifier_Free(identifier);
    return result;
}

// Java: static <V, T extends V> T register(Registry<V> registry, Identifier id, T value)
void *LIBMATTI_MC_Registry_RegisterWithIdentifier(LIBMATTI_MC_MappedRegistry *registry,
                                                  const LIBMATTI_MC_Identifier *identifier, void *value)
{
    LIBMATTI_MC_ResourceKey *resourceKey = LIBMATTI_MC_ResourceKey_Create(registry->key, identifier);
    return LIBMATTI_MC_Registry_Register(registry, resourceKey, value);
}

// Java: static <V, T extends V> T register(Registry<V> registry, ResourceKey<V> key, T value)
void *LIBMATTI_MC_Registry_Register(LIBMATTI_MC_MappedRegistry *registry,
                                    LIBMATTI_MC_ResourceKey *resourceKey, void *value)
{
    LIBMATTI_MC_MappedRegistry_Register(registry, resourceKey, value,
                                        LIBMATTI_MC_RegistrationInfo_BuiltIn());
    return value;
}

// Java: default Optional<T> getOptional(@Nullable Identifier)
void *LIBMATTI_MC_Registry_GetOptional(const LIBMATTI_MC_MappedRegistry *registry,
                                       const LIBMATTI_MC_Identifier *location)
{
    return LIBMATTI_MC_MappedRegistry_GetValue(registry, location);
}

// Java: default T getValueOrThrow(ResourceKey<T>)
void *LIBMATTI_MC_Registry_GetValueOrThrow(const LIBMATTI_MC_MappedRegistry *registry,
                                           const LIBMATTI_MC_ResourceKey *resourceKey)
{
    void *value = LIBMATTI_MC_MappedRegistry_GetValueByKey(registry, resourceKey);
    if (value == NULL)
    {
        char *key = LIBMATTI_MC_ResourceKey_ToString(resourceKey);
        char *registryName = LIBMATTI_MC_Identifier_ToString(registry->key->identifier);
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Missing key in {}: {}", registryName, key);
        free(key);
        free(registryName);
        exit(1);
    }
    return value;
}
