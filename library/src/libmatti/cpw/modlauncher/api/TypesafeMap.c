//
// Port of cpw.mods.modlauncher.api.TypesafeMap.
//

#include "TypesafeMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: static final class Key<T>
struct LIBMATTI_MLA_Key
{
    char *name;
    long long uniqueId;
    const char *clazz; // Java: Class<T> clz
};

// Java: ConcurrentHashMap<Key<Object>, Object> map
typedef struct
{
    LIBMATTI_MLA_Key *key;
    void *value;
} MapEntry;

// Java: ConcurrentHashMap<String, Key<Object>> keys
typedef struct
{
    char *name;
    LIBMATTI_MLA_Key *key;
} KeyIdentifier;

struct LIBMATTI_MLA_TypesafeMap
{
    MapEntry *entries;
    size_t entryCount;
    KeyIdentifier *keys;
    size_t keyCount;
};

// Java: static final class KeyBuilder<T> implements Supplier<Key<T>>
struct LIBMATTI_MLA_KeyBuilder
{
    const char *owner;
    char *name;
    const char *clazz;
    LIBMATTI_MLA_Key *key;
};

// Java: private static final ConcurrentHashMap<Class<?>, TypesafeMap> maps
typedef struct
{
    const char *owner;
    LIBMATTI_MLA_TypesafeMap *map;
} OwnerMap;

// Java: private static final Map<Class<?>, List<KeyBuilder<?>>> keyBuilders
static OwnerMap *ownerMaps = NULL;
static size_t ownerMapCount = 0;

static LIBMATTI_MLA_KeyBuilder **keyBuilders = NULL;
static size_t keyBuilderCount = 0;

// Java: private static final AtomicLong idGenerator
static long long idGenerator = 0;

// ---------------------------------------------------------------------------
// Key
// ---------------------------------------------------------------------------

// Java: static <V> Key<V> getOrCreate(TypesafeMap owner, String name, Class<? super V> clazz)
LIBMATTI_MLA_Key *LIBMATTI_MLA_Key_GetOrCreate(LIBMATTI_MLA_TypesafeMap *owner, const char *name, const char *clazz)
{
    for (size_t i = 0; i < owner->keyCount; i++)
    {
        if (strcmp(owner->keys[i].name, name) != 0) continue;

        LIBMATTI_MLA_Key *result = owner->keys[i].key;

        // Java: if (result.clz != clazz) throw new IllegalArgumentException("Invalid type")
        // (the C port identifies the class by its name)
        if (strcmp(result->clazz, clazz) != 0) return NULL;

        return result;
    }

    LIBMATTI_MLA_Key *key = calloc(1, sizeof(LIBMATTI_MLA_Key));
    if (key == NULL) return NULL;

    key->name = strdup(name);
    key->clazz = clazz;
    key->uniqueId = idGenerator++;

    owner->keys = realloc(owner->keys, sizeof(KeyIdentifier) * (owner->keyCount + 1));
    owner->keys[owner->keyCount].name = strdup(name);
    owner->keys[owner->keyCount].key = key;
    owner->keyCount++;

    return key;
}

// Java: public final String name()
const char *LIBMATTI_MLA_Key_Name(const LIBMATTI_MLA_Key *key)
{
    return key->name;
}

// ---------------------------------------------------------------------------
// TypesafeMap
// ---------------------------------------------------------------------------

// Java: public TypesafeMap()
LIBMATTI_MLA_TypesafeMap *LIBMATTI_MLA_TypesafeMap_New(void)
{
    return calloc(1, sizeof(LIBMATTI_MLA_TypesafeMap));
}

// Java: public TypesafeMap(Class<?> owner)
LIBMATTI_MLA_TypesafeMap *LIBMATTI_MLA_TypesafeMap_NewOwned(const char *owner)
{
    LIBMATTI_MLA_TypesafeMap *map = LIBMATTI_MLA_TypesafeMap_New();
    if (map == NULL) return NULL;

    // Java: keyBuilders.getOrDefault(owner, List.of()).forEach(kb -> kb.buildKey(this))
    for (size_t i = 0; i < keyBuilderCount; i++)
    {
        LIBMATTI_MLA_KeyBuilder *builder = keyBuilders[i];
        if (strcmp(builder->owner, owner) != 0) continue;

        builder->key = LIBMATTI_MLA_Key_GetOrCreate(map, builder->name, builder->clazz);
    }

    // Java: maps.put(owner, this)
    ownerMaps = realloc(ownerMaps, sizeof(OwnerMap) * (ownerMapCount + 1));
    ownerMaps[ownerMapCount].owner = owner;
    ownerMaps[ownerMapCount].map = map;
    ownerMapCount++;

    return map;
}

// Java: public <V> Optional<V> get(Key<V> key) - NULL means Optional.empty()
void *LIBMATTI_MLA_TypesafeMap_Get(const LIBMATTI_MLA_TypesafeMap *map, LIBMATTI_MLA_Key *key)
{
    if (map == NULL || key == NULL) return NULL;

    for (size_t i = 0; i < map->entryCount; i++)
    {
        if (map->entries[i].key == key) return map->entries[i].value;
    }
    return NULL;
}

// Java: public <V> V computeIfAbsent(Key<V> key, Function<? super Key<V>, ? extends V> valueFunction)
void *LIBMATTI_MLA_TypesafeMap_ComputeIfAbsent(LIBMATTI_MLA_TypesafeMap *map, LIBMATTI_MLA_Key *key,
                                               void *(*valueFunction)(LIBMATTI_MLA_Key *key, void *userdata),
                                               void *userdata)
{
    if (map == NULL || key == NULL) return NULL;

    for (size_t i = 0; i < map->entryCount; i++)
    {
        if (map->entries[i].key == key) return map->entries[i].value;
    }

    void *value = valueFunction != NULL ? valueFunction(key, userdata) : NULL;

    // Java: ConcurrentHashMap.computeIfAbsent does not store a null mapping
    if (value == NULL) return NULL;

    map->entries = realloc(map->entries, sizeof(MapEntry) * (map->entryCount + 1));
    map->entries[map->entryCount].key = key;
    map->entries[map->entryCount].value = value;
    map->entryCount++;

    return value;
}

// ---------------------------------------------------------------------------
// KeyBuilder
// ---------------------------------------------------------------------------

// Java: public KeyBuilder(String name, Class<? super T> clazz, Class<?> owner)
LIBMATTI_MLA_KeyBuilder *LIBMATTI_MLA_KeyBuilder_New(const char *name, const char *clazz, const char *owner)
{
    LIBMATTI_MLA_KeyBuilder *builder = calloc(1, sizeof(LIBMATTI_MLA_KeyBuilder));
    if (builder == NULL) return NULL;

    builder->owner = owner;
    builder->name = strdup(name);
    builder->clazz = clazz;

    // Java: keyBuilders.computeIfAbsent(owner, k -> new ArrayList<>()).add(this)
    keyBuilders = realloc(keyBuilders, sizeof(*keyBuilders) * (keyBuilderCount + 1));
    keyBuilders[keyBuilderCount++] = builder;

    return builder;
}

// Java: public Key<T> get()
LIBMATTI_MLA_Key *LIBMATTI_MLA_KeyBuilder_Get(LIBMATTI_MLA_KeyBuilder *builder)
{
    // Java: if (key == null && maps.containsKey(owner)) buildKey(maps.get(owner))
    if (builder->key == NULL)
    {
        for (size_t i = 0; i < ownerMapCount; i++)
        {
            if (strcmp(ownerMaps[i].owner, builder->owner) != 0) continue;

            builder->key = LIBMATTI_MLA_Key_GetOrCreate(ownerMaps[i].map, builder->name, builder->clazz);
            break;
        }
    }

    // Java: if (key == null) throw new NullPointerException("Missing map")
    return builder->key;
}
