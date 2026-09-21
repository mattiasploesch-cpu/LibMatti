#include "libmatti/java/util/HashMap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Java: static final int hash(Object key) -> (h = key.hashCode()) ^ (h >>> 16)
// The port hashes the string like Java's String.hashCode and folds the high bits in.
static size_t string_hash(const char *key)
{
    uint32_t h = 0;
    for (const unsigned char *p = (const unsigned char *) key; *p != '\0'; p++)
        h = h * 31u + *p;
    return (size_t) (h ^ (h >> 16));
}

static void *table_alloc(size_t capacity)
{
    return calloc(capacity, sizeof(LIBMATTI_JU_HashMapEntry *));
}

// Java: final Node<K,V>[] resize() - capacity doubles, starting at 16
static void resize(LIBMATTI_JU_HashMap *map)
{
    size_t newCapacity = map->capacity * 2;
    LIBMATTI_JU_HashMapEntry **newTable = table_alloc(newCapacity);
    if (newTable == NULL)
        return;

    for (size_t i = 0; i < map->capacity; i++)
    {
        LIBMATTI_JU_HashMapEntry *entry = map->table[i];
        while (entry != NULL)
        {
            LIBMATTI_JU_HashMapEntry *next = entry->next;
            size_t index = entry->hash & (newCapacity - 1);
            entry->next = newTable[index];
            newTable[index] = entry;
            entry = next;
        }
    }
    free(map->table);
    map->table = newTable;
    map->capacity = newCapacity;
}

LIBMATTI_JU_HashMap *LIBMATTI_JU_HashMap_New(void)
{
    return LIBMATTI_JU_HashMap_NewWithCapacity(16);
}

LIBMATTI_JU_HashMap *LIBMATTI_JU_HashMap_NewWithCapacity(size_t initialCapacity)
{
    LIBMATTI_JU_HashMap *map = calloc(1, sizeof(LIBMATTI_JU_HashMap));
    size_t capacity = 16;
    while (capacity < initialCapacity)
        capacity <<= 1;
    map->capacity = capacity;
    map->table = table_alloc(capacity);
    return map;
}

void LIBMATTI_JU_HashMap_Free(LIBMATTI_JU_HashMap *map)
{
    if (map == NULL)
        return;
    LIBMATTI_JU_HashMap_Clear(map);
    free(map->table);
    free(map);
}

size_t LIBMATTI_JU_HashMap_Size(const LIBMATTI_JU_HashMap *map)
{
    return map->size;
}

int LIBMATTI_JU_HashMap_IsEmpty(const LIBMATTI_JU_HashMap *map)
{
    return map->size == 0;
}

void *LIBMATTI_JU_HashMap_Get(const LIBMATTI_JU_HashMap *map, const char *key)
{
    if (key == NULL || map->table == NULL)
        return NULL;
    size_t hash = string_hash(key);
    LIBMATTI_JU_HashMapEntry *entry = map->table[hash & (map->capacity - 1)];
    while (entry != NULL)
    {
        if (entry->hash == hash && strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

void *LIBMATTI_JU_HashMap_GetOrDefault(const LIBMATTI_JU_HashMap *map, const char *key, void *defaultValue)
{
    void *value = LIBMATTI_JU_HashMap_Get(map, key);
    return value != NULL ? value : defaultValue;
}

int LIBMATTI_JU_HashMap_ContainsKey(const LIBMATTI_JU_HashMap *map, const char *key)
{
    return LIBMATTI_JU_HashMap_Get(map, key) != NULL;
}

void *LIBMATTI_JU_HashMap_Put(LIBMATTI_JU_HashMap *map, const char *key, void *value)
{
    size_t hash = string_hash(key);
    LIBMATTI_JU_HashMapEntry *entry = map->table[hash & (map->capacity - 1)];
    while (entry != NULL)
    {
        if (entry->hash == hash && strcmp(entry->key, key) == 0)
        {
            void *old = entry->value;
            entry->value = value;
            return old;
        }
        entry = entry->next;
    }

    // Java: putVal - the node goes to the front of its bucket
    LIBMATTI_JU_HashMapEntry *node = malloc(sizeof(LIBMATTI_JU_HashMapEntry));
    if (node == NULL)
        return NULL;
    node->key = strdup(key);
    node->value = value;
    node->hash = hash;
    size_t index = hash & (map->capacity - 1);
    node->next = map->table[index];
    map->table[index] = node;
    map->size++;

    // Java: if (++size > threshold) resize();
    if (map->size > map->capacity * 3 / 4)
        resize(map);
    return NULL;
}

void *LIBMATTI_JU_HashMap_PutIfAbsent(LIBMATTI_JU_HashMap *map, const char *key, void *value)
{
    void *existing = LIBMATTI_JU_HashMap_Get(map, key);
    if (existing != NULL)
        return existing;
    return LIBMATTI_JU_HashMap_Put(map, key, value);
}

void *LIBMATTI_JU_HashMap_ComputeIfAbsent(LIBMATTI_JU_HashMap *map, const char *key,
                                          void *(*function)(const char *key, void *self), void *self)
{
    void *existing = LIBMATTI_JU_HashMap_Get(map, key);
    if (existing != NULL)
        return existing;
    void *computed = function(key, self);
    if (computed != NULL)
        LIBMATTI_JU_HashMap_Put(map, key, computed);
    return computed;
}

void *LIBMATTI_JU_HashMap_Remove(LIBMATTI_JU_HashMap *map, const char *key)
{
    size_t hash = string_hash(key);
    LIBMATTI_JU_HashMapEntry **slot = &map->table[hash & (map->capacity - 1)];
    while (*slot != NULL)
    {
        LIBMATTI_JU_HashMapEntry *entry = *slot;
        if (entry->hash == hash && strcmp(entry->key, key) == 0)
        {
            *slot = entry->next;
            void *old = entry->value;
            free(entry->key);
            free(entry);
            map->size--;
            return old;
        }
        slot = &entry->next;
    }
    return NULL;
}

void LIBMATTI_JU_HashMap_Clear(LIBMATTI_JU_HashMap *map)
{
    for (size_t i = 0; i < map->capacity; i++)
    {
        LIBMATTI_JU_HashMapEntry *entry = map->table[i];
        while (entry != NULL)
        {
            LIBMATTI_JU_HashMapEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
        map->table[i] = NULL;
    }
    map->size = 0;
}

void LIBMATTI_JU_HashMap_ForEach(const LIBMATTI_JU_HashMap *map,
                                 void (*action)(const char *key, void *value, void *self), void *self)
{
    for (size_t i = 0; i < map->capacity; i++)
        for (LIBMATTI_JU_HashMapEntry *entry = map->table[i]; entry != NULL; entry = entry->next)
            action(entry->key, entry->value, self);
}

char **LIBMATTI_JU_HashMap_KeySet(const LIBMATTI_JU_HashMap *map, size_t *count)
{
    char **keys = malloc(sizeof(char *) * (map->size > 0 ? map->size : 1));
    size_t index = 0;
    for (size_t i = 0; i < map->capacity; i++)
        for (LIBMATTI_JU_HashMapEntry *entry = map->table[i]; entry != NULL; entry = entry->next)
            keys[index++] = entry->key;
    *count = index;
    // Note: the key strings are borrowed from the map's entries (freed by
    // Clear/Free); the caller frees only the array.
    return keys;
}
