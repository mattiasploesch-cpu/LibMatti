// Port of net.minecraft.core.component.DataComponentMap.

#include "libmatti/net/minecraft/core/component/DataComponentMap.h"

#include <stdlib.h>

// Java: DataComponentMap.EMPTY - a shared singleton
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Empty(void)
{
    static LIBMATTI_MC_DataComponentMap empty = {0};
    return &empty;
}

LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_New(void)
{
    LIBMATTI_MC_DataComponentMap *map = calloc(1, sizeof(LIBMATTI_MC_DataComponentMap));
    return map;
}

// Java: the Reference2ObjectArrayMap keyed by type identity - the port matches pointers
static ssize_t find_index(const LIBMATTI_MC_DataComponentMap *map, const LIBMATTI_MC_DataComponentType *type)
{
    for (size_t i = 0; i < map->size; i++)
    {
        if (map->types[i] == type)
            return (ssize_t) i;
    }
    return -1;
}

void *LIBMATTI_MC_DataComponentMap_Get(const LIBMATTI_MC_DataComponentMap *map,
                                       const LIBMATTI_MC_DataComponentType *type)
{
    ssize_t index = find_index(map, type);
    return index < 0 ? NULL : map->values[index];
}

bool LIBMATTI_MC_DataComponentMap_Has(const LIBMATTI_MC_DataComponentMap *map,
                                      const LIBMATTI_MC_DataComponentType *type)
{
    return find_index(map, type) >= 0;
}

LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Set(LIBMATTI_MC_DataComponentMap *map,
                                                               const LIBMATTI_MC_DataComponentType *type,
                                                               void *value)
{
    ssize_t index = find_index(map, type);
    if (index >= 0)
    {
        map->values[index] = value;
        return map;
    }
    if (map->size == map->capacity)
    {
        map->capacity = map->capacity > 0 ? map->capacity * 2 : 8;
        map->types = realloc(map->types, sizeof(LIBMATTI_MC_DataComponentType *) * map->capacity);
        map->values = realloc(map->values, sizeof(void *) * map->capacity);
    }
    map->types[map->size] = (LIBMATTI_MC_DataComponentType *) type;
    map->values[map->size] = value;
    map->size++;
    return map;
}

size_t LIBMATTI_MC_DataComponentMap_Size(const LIBMATTI_MC_DataComponentMap *map)
{
    return map->size;
}

bool LIBMATTI_MC_DataComponentMap_IsEmpty(const LIBMATTI_MC_DataComponentMap *map)
{
    return map->size == 0;
}

LIBMATTI_MC_DataComponentType **LIBMATTI_MC_DataComponentMap_KeySet(const LIBMATTI_MC_DataComponentMap *map,
                                                                    size_t *count)
{
    if (count != NULL)
        *count = map->size;
    return map->types;
}

// Java: static DataComponentMap composite(a, b) - b's values win
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Composite(const LIBMATTI_MC_DataComponentMap *a,
                                                                     const LIBMATTI_MC_DataComponentMap *b)
{
    LIBMATTI_MC_DataComponentMap *result = LIBMATTI_MC_DataComponentMap_New();
    for (size_t i = 0; i < a->size; i++)
        LIBMATTI_MC_DataComponentMap_Set(result, a->types[i], a->values[i]);
    for (size_t i = 0; i < b->size; i++)
        LIBMATTI_MC_DataComponentMap_Set(result, b->types[i], b->values[i]);
    return result;
}

LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_DataComponentMap_Copy(const LIBMATTI_MC_DataComponentMap *map)
{
    LIBMATTI_MC_DataComponentMap *copy = LIBMATTI_MC_DataComponentMap_New();
    for (size_t i = 0; i < map->size; i++)
        LIBMATTI_MC_DataComponentMap_Set(copy, map->types[i], map->values[i]);
    return copy;
}

void LIBMATTI_MC_DataComponentMap_Free(LIBMATTI_MC_DataComponentMap *map)
{
    if (map == NULL || map == LIBMATTI_MC_DataComponentMap_Empty())
        return;
    free(map->types);
    free(map->values);
    free(map);
}
