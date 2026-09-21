// Port of the it.unimi.dsi.fastutil.ints collections (open addressing like the
// originals, linear probing).

#include "libmatti/it/unimi/dsi/fastutil/ints/IntCollections.h"

#include <stdlib.h>
#include <string.h>

// Java: it.unimi.dsi.fastutil.HashCommon.mix(key)
static size_t mix(int32_t key)
{
    size_t h = (size_t) key;
    h ^= (h >> 33);
    h *= 0xFF51AFD7ED558CCDULL;
    h ^= (h >> 33);
    h *= 0xC4CEB9FE1A85EC53ULL;
    h ^= (h >> 33);
    return h;
}

// ---------------------------------------------------------------------------
// Int2ObjectMap
// ---------------------------------------------------------------------------

LIBMATTI_FU_Int2ObjectMap *LIBMATTI_FU_Int2ObjectMap_New(void)
{
    LIBMATTI_FU_Int2ObjectMap *map = calloc(1, sizeof(LIBMATTI_FU_Int2ObjectMap));
    map->capacity = 16;
    map->entries = calloc(map->capacity, sizeof(LIBMATTI_FU_Int2ObjectEntry));
    return map;
}

// Java: rehash() - doubles the table
static void rehash(LIBMATTI_FU_Int2ObjectMap *map)
{
    size_t newCapacity = map->capacity * 2;
    LIBMATTI_FU_Int2ObjectEntry *old = map->entries;
    size_t oldCapacity = map->capacity;

    map->entries = calloc(newCapacity, sizeof(LIBMATTI_FU_Int2ObjectEntry));
    map->capacity = newCapacity;

    for (size_t i = 0; i < oldCapacity; i++)
    {
        if (!old[i].used)
            continue;
        size_t index = mix(old[i].key) & (newCapacity - 1);
        while (map->entries[index].used)
            index = (index + 1) & (newCapacity - 1);
        map->entries[index] = old[i];
    }
    free(old);
}

void LIBMATTI_FU_Int2ObjectMap_Free(LIBMATTI_FU_Int2ObjectMap *map)
{
    if (map == NULL)
        return;
    free(map->entries);
    free(map);
}

void *LIBMATTI_FU_Int2ObjectMap_Put(LIBMATTI_FU_Int2ObjectMap *map, int32_t key, void *value)
{
    if (map->size >= map->capacity * 3 / 4)
        rehash(map);

    size_t index = mix(key) & (map->capacity - 1);
    while (map->entries[index].used)
    {
        if (map->entries[index].key == key)
        {
            void *old = map->entries[index].value;
            map->entries[index].value = value;
            return old;
        }
        index = (index + 1) & (map->capacity - 1);
    }
    map->entries[index].used = 1;
    map->entries[index].key = key;
    map->entries[index].value = value;
    map->size++;
    return NULL;
}

void *LIBMATTI_FU_Int2ObjectMap_Get(const LIBMATTI_FU_Int2ObjectMap *map, int32_t key)
{
    size_t index = mix(key) & (map->capacity - 1);
    while (map->entries[index].used)
    {
        if (map->entries[index].key == key)
            return map->entries[index].value;
        index = (index + 1) & (map->capacity - 1);
    }
    return NULL;
}

void *LIBMATTI_FU_Int2ObjectMap_Remove(LIBMATTI_FU_Int2ObjectMap *map, int32_t key)
{
    size_t index = mix(key) & (map->capacity - 1);
    while (map->entries[index].used)
    {
        if (map->entries[index].key == key)
        {
            void *old = map->entries[index].value;
            map->entries[index].used = 0;
            map->entries[index].value = NULL;
            map->size--;
            return old;
        }
        index = (index + 1) & (map->capacity - 1);
    }
    return NULL;
}

int LIBMATTI_FU_Int2ObjectMap_ContainsKey(const LIBMATTI_FU_Int2ObjectMap *map, int32_t key)
{
    size_t index = mix(key) & (map->capacity - 1);
    while (map->entries[index].used)
    {
        if (map->entries[index].key == key)
            return 1;
        index = (index + 1) & (map->capacity - 1);
    }
    return 0;
}

size_t LIBMATTI_FU_Int2ObjectMap_Size(const LIBMATTI_FU_Int2ObjectMap *map)
{
    return map->size;
}

void LIBMATTI_FU_Int2ObjectMap_ForEach(const LIBMATTI_FU_Int2ObjectMap *map,
                                       void (*action)(int32_t key, void *value, void *self), void *self)
{
    for (size_t i = 0; i < map->capacity; i++)
        if (map->entries[i].used)
            action(map->entries[i].key, map->entries[i].value, self);
}

// ---------------------------------------------------------------------------
// IntOpenHashSet
// ---------------------------------------------------------------------------

LIBMATTI_FU_IntOpenHashSet *LIBMATTI_FU_IntOpenHashSet_New(void)
{
    LIBMATTI_FU_IntOpenHashSet *set = calloc(1, sizeof(LIBMATTI_FU_IntOpenHashSet));
    set->capacity = 16;
    set->keys = calloc(set->capacity, sizeof(int32_t));
    set->used = calloc(set->capacity, 1);
    return set;
}

static void set_rehash(LIBMATTI_FU_IntOpenHashSet *set)
{
    size_t newCapacity = set->capacity * 2;
    int32_t *oldKeys = set->keys;
    unsigned char *oldUsed = set->used;
    size_t oldCapacity = set->capacity;

    set->keys = calloc(newCapacity, sizeof(int32_t));
    set->used = calloc(newCapacity, 1);
    set->capacity = newCapacity;
    set->size = 0;

    for (size_t i = 0; i < oldCapacity; i++)
        if (oldUsed[i])
            LIBMATTI_FU_IntOpenHashSet_Add(set, oldKeys[i]);
    free(oldKeys);
    free(oldUsed);
}

void LIBMATTI_FU_IntOpenHashSet_Free(LIBMATTI_FU_IntOpenHashSet *set)
{
    if (set == NULL)
        return;
    free(set->keys);
    free(set->used);
    free(set);
}

int LIBMATTI_FU_IntOpenHashSet_Add(LIBMATTI_FU_IntOpenHashSet *set, int32_t key)
{
    if (set->size >= set->capacity * 3 / 4)
        set_rehash(set);

    size_t index = mix(key) & (set->capacity - 1);
    while (set->used[index])
    {
        if (set->keys[index] == key)
            return 0;
        index = (index + 1) & (set->capacity - 1);
    }
    set->used[index] = 1;
    set->keys[index] = key;
    set->size++;
    return 1;
}

int LIBMATTI_FU_IntOpenHashSet_Contains(const LIBMATTI_FU_IntOpenHashSet *set, int32_t key)
{
    size_t index = mix(key) & (set->capacity - 1);
    while (set->used[index])
    {
        if (set->keys[index] == key)
            return 1;
        index = (index + 1) & (set->capacity - 1);
    }
    return 0;
}

int LIBMATTI_FU_IntOpenHashSet_Remove(LIBMATTI_FU_IntOpenHashSet *set, int32_t key)
{
    size_t index = mix(key) & (set->capacity - 1);
    while (set->used[index])
    {
        if (set->keys[index] == key)
        {
            set->used[index] = 0;
            set->size--;
            return 1;
        }
        index = (index + 1) & (set->capacity - 1);
    }
    return 0;
}

size_t LIBMATTI_FU_IntOpenHashSet_Size(const LIBMATTI_FU_IntOpenHashSet *set)
{
    return set->size;
}

// ---------------------------------------------------------------------------
// IntArrayList
// ---------------------------------------------------------------------------

LIBMATTI_FU_IntArrayList *LIBMATTI_FU_IntArrayList_New(void)
{
    LIBMATTI_FU_IntArrayList *list = calloc(1, sizeof(LIBMATTI_FU_IntArrayList));
    list->capacity = 16;
    list->elements = malloc(sizeof(int32_t) * list->capacity);
    return list;
}

void LIBMATTI_FU_IntArrayList_Free(LIBMATTI_FU_IntArrayList *list)
{
    if (list == NULL)
        return;
    free(list->elements);
    free(list);
}

void LIBMATTI_FU_IntArrayList_Add(LIBMATTI_FU_IntArrayList *list, int32_t value)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        list->elements = realloc(list->elements, sizeof(int32_t) * list->capacity);
    }
    list->elements[list->size++] = value;
}

int32_t LIBMATTI_FU_IntArrayList_Get(const LIBMATTI_FU_IntArrayList *list, size_t index)
{
    if (index >= list->size)
        return 0;
    return list->elements[index];
}

size_t LIBMATTI_FU_IntArrayList_Size(const LIBMATTI_FU_IntArrayList *list)
{
    return list->size;
}

int32_t *LIBMATTI_FU_IntArrayList_ToArray(const LIBMATTI_FU_IntArrayList *list, size_t *count)
{
    int32_t *array = malloc(sizeof(int32_t) * (list->size > 0 ? list->size : 1));
    memcpy(array, list->elements, sizeof(int32_t) * list->size);
    *count = list->size;
    return array;
}
