// Port of net.minecraft.core.MappedRegistry (with the DefaultedMappedRegistry fallbacks).

#include "libmatti/net/minecraft/core/MappedRegistry.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void map_put(void ***keys, void ***values, size_t *count, size_t *capacity,
                    void *key, void *value)
{
    if (*count == *capacity)
    {
        *capacity = *capacity > 0 ? *capacity * 2 : 8;
        *keys = realloc(*keys, sizeof(void *) * *capacity);
        *values = realloc(*values, sizeof(void *) * *capacity);
    }
    (*keys)[*count] = key;
    (*values)[*count] = value;
    (*count)++;
}

static void *map_get(void **keys, void **values, size_t count, const void *key,
                     int (*equals)(const void *a, const void *b))
{
    for (size_t i = 0; i < count; i++)
        if (equals(keys[i], key))
            return values[i];
    return NULL;
}

static int pointer_equals(const void *a, const void *b)
{
    return a == b;
}

static int identifier_equals_entry(const void *a, const void *b)
{
    return LIBMATTI_MC_Identifier_Equals((const LIBMATTI_MC_Identifier *) a,
                                         (const LIBMATTI_MC_Identifier *) b);
}

static int resource_key_equals_entry(const void *a, const void *b)
{
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    // Java: ResourceKey equality is the interned identity; the port compares the pair so
    // keys built through different paths still find the same entry
    return LIBMATTI_MC_Identifier_Equals(((const LIBMATTI_MC_ResourceKey *) a)->registryName,
                                         ((const LIBMATTI_MC_ResourceKey *) b)->registryName) &&
           LIBMATTI_MC_Identifier_Equals(((const LIBMATTI_MC_ResourceKey *) a)->identifier,
                                         ((const LIBMATTI_MC_ResourceKey *) b)->identifier);
}

// Java: MappedRegistry.RegistryBootstrap - not part of this class

// Java: public MappedRegistry(ResourceKey, Lifecycle, boolean intrusive)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_MappedRegistry_New(LIBMATTI_MC_ResourceKey *key,
                                                            LIBMATTI_MJS_Lifecycle lifecycle,
                                                            int intrusiveHolders)
{
    LIBMATTI_MC_MappedRegistry *registry = calloc(1, sizeof(LIBMATTI_MC_MappedRegistry));
    registry->key = key;
    registry->registryLifecycle = lifecycle;
    registry->intrusiveHolders = intrusiveHolders;
    return registry;
}

// The port's DefaultedMappedRegistry constructor
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_DefaultedMappedRegistry_New(const char *defaultKey,
                                                                    LIBMATTI_MC_ResourceKey *key,
                                                                    LIBMATTI_MJS_Lifecycle lifecycle,
                                                                    int intrusiveHolders)
{
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_MappedRegistry_New(key, lifecycle, intrusiveHolders);
    registry->defaultKey = strdup(defaultKey);
    return registry;
}

void LIBMATTI_MC_MappedRegistry_Free(LIBMATTI_MC_MappedRegistry *registry)
{
    if (registry == NULL) return;
    // The ResourceKeys are interned (owned by the intern table); the holder records, the
    // identifier copies in byLocation and the info array are this registry's own.
    for (size_t i = 0; i < registry->byIdCount; i++)
        free(registry->byId[i]);
    free(registry->byId);
    free(registry->toIdKeys);
    free(registry->toIdValues);
    for (size_t i = 0; i < registry->byLocation.count; i++)
        LIBMATTI_MC_Identifier_Free(registry->byLocation.keys[i]);
    free(registry->byLocation.keys);
    free(registry->byLocation.values);
    free(registry->byKey.keys);
    free(registry->byKey.values);
    free(registry->byValue.keys);
    free(registry->byValue.values);
    free(registry->registrationInfos.keys);
    free(registry->registrationInfos.values);
    free(registry->defaultKey);
    free(registry);
}

// Java: public ResourceKey<? extends Registry<T>> key()
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_MappedRegistry_Key(const LIBMATTI_MC_MappedRegistry *registry)
{
    return registry->key;
}

// Java: private void validateWrite(ResourceKey key) - throw new IllegalStateException
void LIBMATTI_MC_MappedRegistry_ValidateWrite(const LIBMATTI_MC_MappedRegistry *registry)
{
    if (registry->frozen)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Registry is already frozen");
        exit(1);
    }
}

// Java: Holder.Reference.createStandAlone(this, key) - unbound holder with a known key
static LIBMATTI_MC_HolderReference *create_stand_alone(LIBMATTI_MC_MappedRegistry *registry,
                                                       LIBMATTI_MC_ResourceKey *resourceKey)
{
    LIBMATTI_MC_HolderReference *reference = calloc(1, sizeof(LIBMATTI_MC_HolderReference));
    reference->registry = registry;
    reference->key = resourceKey;
    return reference;
}

// Java: public Holder.Reference<T> register(ResourceKey<T> key, T value, RegistrationInfo info)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_Register(LIBMATTI_MC_MappedRegistry *registry,
                                                                 LIBMATTI_MC_ResourceKey *resourceKey,
                                                                 void *value,
                                                                 LIBMATTI_MC_RegistrationInfo info)
{
    LIBMATTI_MC_MappedRegistry_ValidateWrite(registry);

    if (LIBMATTI_MC_MappedRegistry_ContainsLocation(registry, resourceKey->identifier))
    {
        char *key = LIBMATTI_MC_ResourceKey_ToString(resourceKey);
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Adding duplicate key '{}' to registry", key);
        free(key);
        exit(1);
    }
    if (LIBMATTI_MC_MappedRegistry_GetId(registry, value) != -1)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Adding duplicate value to registry");
        exit(1);
    }

    LIBMATTI_MC_HolderReference *reference = NULL;
    if (registry->intrusiveHolders)
    {
        // Java: the intrusive holder was created through createIntrusiveHolder(value) before
        // the register call; the port binds it here (the holder for the value is the registry's)
        reference = map_get((void **) registry->byValue.keys, (void **) registry->byValue.values,
                            registry->byValue.count, value, pointer_equals);
    }
    if (reference == NULL)
        reference = create_stand_alone(registry, resourceKey);

    reference->registry = registry;
    reference->key = resourceKey;

    LIBMATTI_MC_Identifier *location =
        LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_GetNamespace(resourceKey->identifier),
                                   LIBMATTI_MC_Identifier_GetPath(resourceKey->identifier));
    map_put((void ***) &registry->byKey.keys, (void ***) &registry->byKey.values,
            &registry->byKey.count, &registry->byKey.capacity, resourceKey, reference);
    map_put((void ***) &registry->byLocation.keys, (void ***) &registry->byLocation.values,
            &registry->byLocation.count, &registry->byLocation.capacity, location, reference);
    map_put((void ***) &registry->byValue.keys, (void ***) &registry->byValue.values,
            &registry->byValue.count, &registry->byValue.capacity, (void *) value, reference);

    int id = (int) registry->byIdCount;
    if (registry->byIdCount == registry->byIdCapacity)
    {
        registry->byIdCapacity = registry->byIdCapacity > 0 ? registry->byIdCapacity * 2 : 256;
        registry->byId = realloc(registry->byId, sizeof(LIBMATTI_MC_HolderReference *) * registry->byIdCapacity);
    }
    registry->byId[registry->byIdCount++] = reference;

    if (registry->toIdCount == registry->toIdCapacity)
    {
        registry->toIdCapacity = registry->toIdCapacity > 0 ? registry->toIdCapacity * 2 : 256;
        registry->toIdKeys = realloc(registry->toIdKeys, sizeof(void *) * registry->toIdCapacity);
        registry->toIdValues = realloc(registry->toIdValues, sizeof(int) * registry->toIdCapacity);
    }
    registry->toIdKeys[registry->toIdCount] = value;
    registry->toIdValues[registry->toIdCount] = id;
    registry->toIdCount++;

    if (registry->registrationInfos.count == registry->registrationInfos.capacity)
    {
        registry->registrationInfos.capacity =
            registry->registrationInfos.capacity > 0 ? registry->registrationInfos.capacity * 2 : 8;
        registry->registrationInfos.keys =
            realloc(registry->registrationInfos.keys,
                    sizeof(LIBMATTI_MC_ResourceKey *) * registry->registrationInfos.capacity);
        registry->registrationInfos.values =
            realloc(registry->registrationInfos.values,
                    sizeof(LIBMATTI_MC_RegistrationInfo) * registry->registrationInfos.capacity);
    }
    registry->registrationInfos.keys[registry->registrationInfos.count] = resourceKey;
    registry->registrationInfos.values[registry->registrationInfos.count] = info;
    registry->registrationInfos.count++;

    registry->registryLifecycle =
        LIBMATTI_MJS_Lifecycle_Add(registry->registryLifecycle, info.lifecycle);
    return reference;
}

// Java: @Nullable public Identifier getKey(T value)
LIBMATTI_MC_Identifier *LIBMATTI_MC_MappedRegistry_GetKey(const LIBMATTI_MC_MappedRegistry *registry, const void *value)
{
    LIBMATTI_MC_HolderReference *reference = map_get((void **) registry->byValue.keys,
                                                     (void **) registry->byValue.values,
                                                     registry->byValue.count, value, pointer_equals);
    return reference != NULL && reference->key != NULL ? reference->key->identifier : NULL;
}

// Java: public Optional<ResourceKey<T>> getResourceKey(T value)
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_MappedRegistry_GetResourceKey(const LIBMATTI_MC_MappedRegistry *registry,
                                                                   const void *value)
{
    LIBMATTI_MC_HolderReference *reference = map_get((void **) registry->byValue.keys,
                                                     (void **) registry->byValue.values,
                                                     registry->byValue.count, value, pointer_equals);
    return reference != NULL ? reference->key : NULL;
}

// Java: public int getId(@Nullable T value) - the map's defaultReturnValue(-1)
int LIBMATTI_MC_MappedRegistry_GetId(const LIBMATTI_MC_MappedRegistry *registry, const void *value)
{
    for (size_t i = 0; i < registry->toIdCount; i++)
        if (registry->toIdKeys[i] == value)
            return registry->toIdValues[i];
    return -1;
}

// Java: @Nullable public T getValue(@Nullable ResourceKey<T> key)
void *LIBMATTI_MC_MappedRegistry_GetValueByKey(const LIBMATTI_MC_MappedRegistry *registry,
                                               const LIBMATTI_MC_ResourceKey *resourceKey)
{
    LIBMATTI_MC_HolderReference *reference = map_get((void **) registry->byKey.keys,
                                                     (void **) registry->byKey.values,
                                                     registry->byKey.count, resourceKey,
                                                     resource_key_equals_entry);
    return reference != NULL ? reference->value : NULL;
}

// Java: @Nullable public T getValue(@Nullable Identifier location)
void *LIBMATTI_MC_MappedRegistry_GetValue(const LIBMATTI_MC_MappedRegistry *registry,
                                          const LIBMATTI_MC_Identifier *location)
{
    LIBMATTI_MC_HolderReference *reference = map_get((void **) registry->byLocation.keys,
                                                     (void **) registry->byLocation.values,
                                                     registry->byLocation.count, location,
                                                     identifier_equals_entry);
    return reference != NULL ? reference->value : NULL;
}

// Java: @Nullable public T byId(int id)
void *LIBMATTI_MC_MappedRegistry_ById(const LIBMATTI_MC_MappedRegistry *registry, int id)
{
    if (id >= 0 && (size_t) id < registry->byIdCount)
        return registry->byId[id]->value;
    return NULL;
}

// Java: public Optional<Holder.Reference<T>> get(int id)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderById(const LIBMATTI_MC_MappedRegistry *registry, int id)
{
    if (id >= 0 && (size_t) id < registry->byIdCount)
        return registry->byId[id];
    return NULL;
}

// Java: public Optional<Holder.Reference<T>> get(Identifier location)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderByLocation(const LIBMATTI_MC_MappedRegistry *registry,
                                                                            const LIBMATTI_MC_Identifier *location)
{
    return map_get((void **) registry->byLocation.keys, (void **) registry->byLocation.values,
                   registry->byLocation.count, location, identifier_equals_entry);
}

// Java: public Optional<Holder.Reference<T>> get(ResourceKey<T> key)
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetHolderByKey(const LIBMATTI_MC_MappedRegistry *registry,
                                                                       const LIBMATTI_MC_ResourceKey *resourceKey)
{
    return map_get((void **) registry->byKey.keys, (void **) registry->byKey.values,
                   registry->byKey.count, resourceKey, resource_key_equals_entry);
}

// Java: public Optional<Holder.Reference<T>> getAny()
LIBMATTI_MC_HolderReference *LIBMATTI_MC_MappedRegistry_GetAny(const LIBMATTI_MC_MappedRegistry *registry)
{
    return registry->byIdCount > 0 ? registry->byId[0] : NULL;
}

// Java: public int size()
int LIBMATTI_MC_MappedRegistry_Size(const LIBMATTI_MC_MappedRegistry *registry)
{
    return (int) registry->byKey.count;
}

// Java: public boolean isEmpty()
int LIBMATTI_MC_MappedRegistry_IsEmpty(const LIBMATTI_MC_MappedRegistry *registry)
{
    return registry->byKey.count == 0;
}

// Java: public Optional<RegistrationInfo> registrationInfo(ResourceKey<T> key)
const LIBMATTI_MC_RegistrationInfo *LIBMATTI_MC_MappedRegistry_RegistrationInfo(const LIBMATTI_MC_MappedRegistry *registry,
                                                                                const LIBMATTI_MC_ResourceKey *resourceKey)
{
    for (size_t i = 0; i < registry->registrationInfos.count; i++)
        if (resource_key_equals_entry(registry->registrationInfos.keys[i], resourceKey))
            return &registry->registrationInfos.values[i];
    return NULL;
}

// Java: public Lifecycle registryLifecycle()
LIBMATTI_MJS_Lifecycle LIBMATTI_MC_MappedRegistry_RegistryLifecycle(const LIBMATTI_MC_MappedRegistry *registry)
{
    return registry->registryLifecycle;
}

// Java: public Iterator<T> iterator()
LIBMATTI_MC_HolderReference **LIBMATTI_MC_MappedRegistry_Iterators(const LIBMATTI_MC_MappedRegistry *registry,
                                                                   size_t *count)
{
    *count = registry->byIdCount;
    return registry->byId;
}

// Java: public Set<Identifier> keySet()
LIBMATTI_MC_Identifier **LIBMATTI_MC_MappedRegistry_KeySet(const LIBMATTI_MC_MappedRegistry *registry, size_t *count)
{
    *count = registry->byLocation.count;
    return registry->byLocation.keys;
}

// Java: public Set<ResourceKey<T>> registryKeySet()
LIBMATTI_MC_ResourceKey **LIBMATTI_MC_MappedRegistry_RegistryKeySet(const LIBMATTI_MC_MappedRegistry *registry,
                                                                    size_t *count)
{
    *count = registry->byKey.count;
    return registry->byKey.keys;
}

// Java: public boolean containsKey(Identifier location)
int LIBMATTI_MC_MappedRegistry_ContainsLocation(const LIBMATTI_MC_MappedRegistry *registry,
                                                const LIBMATTI_MC_Identifier *location)
{
    return map_get((void **) registry->byLocation.keys, (void **) registry->byLocation.values,
                   registry->byLocation.count, location, identifier_equals_entry) != NULL;
}

// Java: public boolean containsKey(ResourceKey<T> key)
int LIBMATTI_MC_MappedRegistry_ContainsKey(const LIBMATTI_MC_MappedRegistry *registry,
                                           const LIBMATTI_MC_ResourceKey *resourceKey)
{
    return map_get((void **) registry->byKey.keys, (void **) registry->byKey.values,
                   registry->byKey.count, resourceKey, resource_key_equals_entry) != NULL;
}

// Java: public Registry<T> freeze() - bindValue for every entry, abort on unbound holders
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_MappedRegistry_Freeze(LIBMATTI_MC_MappedRegistry *registry)
{
    if (registry->frozen)
        return registry;

    registry->frozen = 1;
    // Java: this.byValue.forEach((value, reference) -> reference.bindValue(value))
    for (size_t i = 0; i < registry->byValue.count; i++)
    {
        LIBMATTI_MC_HolderReference *reference = registry->byValue.values[i];
        reference->value = registry->byValue.keys[i];
        reference->bound = 1;
    }

    // Java: collect the unbound holders' identifiers, sorted, and throw
    for (size_t i = 0; i < registry->byKey.count; i++)
    {
        if (!registry->byKey.values[i]->bound)
        {
            char *key = LIBMATTI_MC_ResourceKey_ToString(registry->byKey.keys[i]);
            LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
            LIBMATTI_ML_Logger_Error(logger, NULL, "Unbound values in registry {}: {}", key);
            free(key);
            exit(1);
        }
    }
    return registry;
}

// Java: void unfreeze(boolean usingWrittenKeys)
void LIBMATTI_MC_MappedRegistry_Unfreeze(LIBMATTI_MC_MappedRegistry *registry)
{
    registry->frozen = 0;
}

// Java: DefaultedRegistry.getDefaultKey()
const char *LIBMATTI_MC_MappedRegistry_GetDefaultKey(const LIBMATTI_MC_MappedRegistry *registry)
{
    return registry->defaultKey;
}

// Java: @NonNull public T getValue(@Nullable Identifier) - the default value when missing
void *LIBMATTI_MC_MappedRegistry_GetValueOrDefault(const LIBMATTI_MC_MappedRegistry *registry,
                                                   const LIBMATTI_MC_Identifier *location)
{
    LIBMATTI_MC_Identifier *defaultKey = LIBMATTI_MC_Identifier_Parse(registry->defaultKey);
    void *value = LIBMATTI_MC_MappedRegistry_GetValue(registry, location);
    if (value == NULL)
        value = LIBMATTI_MC_MappedRegistry_GetValue(registry, defaultKey);
    LIBMATTI_MC_Identifier_Free(defaultKey);
    return value;
}

// Java: @NonNull public T byId(int) - the default value when out of bounds
void *LIBMATTI_MC_MappedRegistry_ByIdOrDefault(const LIBMATTI_MC_MappedRegistry *registry, int id)
{
    void *value = LIBMATTI_MC_MappedRegistry_ById(registry, id);
    if (value != NULL)
        return value;
    LIBMATTI_MC_Identifier *defaultKey = LIBMATTI_MC_Identifier_Parse(registry->defaultKey);
    value = LIBMATTI_MC_MappedRegistry_GetValue(registry, defaultKey);
    LIBMATTI_MC_Identifier_Free(defaultKey);
    return value;
}
