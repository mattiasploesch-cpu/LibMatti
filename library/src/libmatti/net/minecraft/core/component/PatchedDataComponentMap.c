// Port of net.minecraft.core.component.PatchedDataComponentMap and DataComponentPatch.

#include "libmatti/net/minecraft/core/component/PatchedDataComponentMap.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------
// DataComponentPatch
// ---------------------------------------------------------------------------

LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_DataComponentPatch_Empty(void)
{
    static LIBMATTI_MC_DataComponentPatch empty = {0};
    return &empty;
}

LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_DataComponentPatch_New(void)
{
    LIBMATTI_MC_DataComponentPatch *patch = calloc(1, sizeof(LIBMATTI_MC_DataComponentPatch));
    return patch;
}

static ssize_t patch_find(const LIBMATTI_MC_DataComponentPatch *patch, const LIBMATTI_MC_DataComponentType *type)
{
    for (size_t i = 0; i < patch->size; i++)
    {
        if (patch->types[i] == type)
            return (ssize_t) i;
    }
    return -1;
}

static void patch_put(LIBMATTI_MC_DataComponentPatch *patch, const LIBMATTI_MC_DataComponentType *type,
                      bool present, void *value)
{
    ssize_t index = patch_find(patch, type);
    if (index >= 0)
    {
        patch->present[index] = present;
        patch->values[index] = value;
        return;
    }
    if (patch->size == patch->capacity)
    {
        patch->capacity = patch->capacity > 0 ? patch->capacity * 2 : 4;
        patch->types = realloc(patch->types, sizeof(LIBMATTI_MC_DataComponentType *) * patch->capacity);
        patch->present = realloc(patch->present, sizeof(bool) * patch->capacity);
        patch->values = realloc(patch->values, sizeof(void *) * patch->capacity);
    }
    patch->types[patch->size] = (LIBMATTI_MC_DataComponentType *) type;
    patch->present[patch->size] = present;
    patch->values[patch->size] = value;
    patch->size++;
}

// Java: map.put(type, Optional.ofNullable(value))
void LIBMATTI_MC_DataComponentPatch_Set(LIBMATTI_MC_DataComponentPatch *patch,
                                        const LIBMATTI_MC_DataComponentType *type, void *value)
{
    patch_put(patch, type, value != NULL, value);
}

// Java: map.put(type, Optional.empty())
void LIBMATTI_MC_DataComponentPatch_Remove(LIBMATTI_MC_DataComponentPatch *patch,
                                           const LIBMATTI_MC_DataComponentType *type)
{
    patch_put(patch, type, false, NULL);
}

bool LIBMATTI_MC_DataComponentPatch_IsEmpty(const LIBMATTI_MC_DataComponentPatch *patch)
{
    return patch->size == 0;
}

void LIBMATTI_MC_DataComponentPatch_Free(LIBMATTI_MC_DataComponentPatch *patch)
{
    if (patch == NULL || patch == LIBMATTI_MC_DataComponentPatch_Empty())
        return;
    free(patch->types);
    free(patch->present);
    free(patch->values);
    free(patch);
}

// ---------------------------------------------------------------------------
// PatchedDataComponentMap
// ---------------------------------------------------------------------------

LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_New(
        const LIBMATTI_MC_DataComponentMap *prototype)
{
    LIBMATTI_MC_PatchedDataComponentMap *map = calloc(1, sizeof(LIBMATTI_MC_PatchedDataComponentMap));
    map->prototype = prototype;
    // Java: this(prototype, emptyMap, true) - copy-on-write until the first mutation
    map->copyOnWrite = true;
    return map;
}

// Java: private void ensureMapOwnership()
static void ensure_map_ownership(LIBMATTI_MC_PatchedDataComponentMap *map)
{
    if (map->copyOnWrite)
    {
        map->copyOnWrite = false;
    }
}

static ssize_t patch_index(const LIBMATTI_MC_PatchedDataComponentMap *map, const LIBMATTI_MC_DataComponentType *type)
{
    for (size_t i = 0; i < map->patchSize; i++)
    {
        if (map->patchTypes[i] == type)
            return (ssize_t) i;
    }
    return -1;
}

static void patch_entry_put(LIBMATTI_MC_PatchedDataComponentMap *map, const LIBMATTI_MC_DataComponentType *type,
                            bool present, void *value)
{
    ensure_map_ownership(map);
    ssize_t index = patch_index(map, type);
    if (index >= 0)
    {
        map->patchPresent[index] = present;
        map->patchValues[index] = value;
        return;
    }
    if (map->patchSize == map->patchCapacity)
    {
        map->patchCapacity = map->patchCapacity > 0 ? map->patchCapacity * 2 : 4;
        map->patchTypes = realloc(map->patchTypes, sizeof(LIBMATTI_MC_DataComponentType *) * map->patchCapacity);
        map->patchPresent = realloc(map->patchPresent, sizeof(bool) * map->patchCapacity);
        map->patchValues = realloc(map->patchValues, sizeof(void *) * map->patchCapacity);
    }
    map->patchTypes[map->patchSize] = (LIBMATTI_MC_DataComponentType *) type;
    map->patchPresent[map->patchSize] = present;
    map->patchValues[map->patchSize] = value;
    map->patchSize++;
}

static void patch_entry_remove(LIBMATTI_MC_PatchedDataComponentMap *map, const LIBMATTI_MC_DataComponentType *type)
{
    ensure_map_ownership(map);
    ssize_t index = patch_index(map, type);
    if (index < 0)
        return;
    for (size_t i = (size_t) index; i + 1 < map->patchSize; i++)
    {
        map->patchTypes[i] = map->patchTypes[i + 1];
        map->patchPresent[i] = map->patchPresent[i + 1];
        map->patchValues[i] = map->patchValues[i + 1];
    }
    map->patchSize--;
}

// Java: public static PatchedDataComponentMap fromPatch(DataComponentMap, DataComponentPatch)
// Java's isPatchSanitized fast path shares the patch map; the port always copies the
// entries (same observable behaviour, simpler ownership).
LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_FromPatch(
        const LIBMATTI_MC_DataComponentMap *prototype, const LIBMATTI_MC_DataComponentPatch *patch)
{
    LIBMATTI_MC_PatchedDataComponentMap *map = LIBMATTI_MC_PatchedDataComponentMap_New(prototype);
    for (size_t i = 0; i < patch->size; i++)
        patch_entry_put(map, patch->types[i], patch->present[i], patch->values[i]);
    return map;
}

// Java: public <T> @Nullable T get(DataComponentType<? extends T>)
void *LIBMATTI_MC_PatchedDataComponentMap_Get(const LIBMATTI_MC_PatchedDataComponentMap *map,
                                              const LIBMATTI_MC_DataComponentType *type)
{
    ssize_t index = patch_index(map, type);
    if (index >= 0)
        return map->patchPresent[index] ? map->patchValues[index] : NULL;
    return LIBMATTI_MC_DataComponentMap_Get(map->prototype, type);
}

// Java: public boolean hasNonDefault(DataComponentType<?>)
bool LIBMATTI_MC_PatchedDataComponentMap_HasNonDefault(const LIBMATTI_MC_PatchedDataComponentMap *map,
                                                       const LIBMATTI_MC_DataComponentType *type)
{
    return patch_index(map, type) >= 0;
}

// Java: public <T> @Nullable T set(DataComponentType<T>, @Nullable T) - the value equal
// to the prototype's drops back out of the patch, like Java's Objects.equals check
void *LIBMATTI_MC_PatchedDataComponentMap_Set(LIBMATTI_MC_PatchedDataComponentMap *map,
                                              const LIBMATTI_MC_DataComponentType *type, void *value)
{
    ensure_map_ownership(map);
    void *prototypeValue = LIBMATTI_MC_DataComponentMap_Get(map->prototype, type);
    void *previous = LIBMATTI_MC_PatchedDataComponentMap_Get(map, type);
    if (value == prototypeValue)
    {
        patch_entry_remove(map, type);
    }
    else
    {
        patch_entry_put(map, type, value != NULL, value);
    }
    return previous;
}

// Java: public <T> @Nullable T remove(DataComponentType<? extends T>)
void *LIBMATTI_MC_PatchedDataComponentMap_Remove(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                 const LIBMATTI_MC_DataComponentType *type)
{
    ensure_map_ownership(map);
    void *prototypeValue = LIBMATTI_MC_DataComponentMap_Get(map->prototype, type);
    void *previous = LIBMATTI_MC_PatchedDataComponentMap_Get(map, type);
    if (prototypeValue != NULL)
    {
        patch_entry_put(map, type, false, NULL);
    }
    else
    {
        patch_entry_remove(map, type);
    }
    return previous;
}

// Java: public void clearPatch()
void LIBMATTI_MC_PatchedDataComponentMap_ClearPatch(LIBMATTI_MC_PatchedDataComponentMap *map)
{
    ensure_map_ownership(map);
    map->patchSize = 0;
}

// Java: public void applyPatch(DataComponentPatch) - applyPatch(type, optional) per entry
void LIBMATTI_MC_PatchedDataComponentMap_ApplyPatch(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                    const LIBMATTI_MC_DataComponentPatch *patch)
{
    for (size_t i = 0; i < patch->size; i++)
    {
        const LIBMATTI_MC_DataComponentType *type = patch->types[i];
        void *prototypeValue = LIBMATTI_MC_DataComponentMap_Get(map->prototype, type);
        if (patch->present[i])
        {
            if (patch->values[i] == prototypeValue)
                patch_entry_remove(map, type);
            else
                patch_entry_put(map, type, true, patch->values[i]);
        }
        else if (prototypeValue != NULL)
        {
            patch_entry_put(map, type, false, NULL);
        }
        else
        {
            patch_entry_remove(map, type);
        }
    }
}

// Java: public void restorePatch(DataComponentPatch) - clear + putAll
void LIBMATTI_MC_PatchedDataComponentMap_RestorePatch(LIBMATTI_MC_PatchedDataComponentMap *map,
                                                      const LIBMATTI_MC_DataComponentPatch *patch)
{
    ensure_map_ownership(map);
    map->patchSize = 0;
    for (size_t i = 0; i < patch->size; i++)
        patch_entry_put(map, patch->types[i], patch->present[i], patch->values[i]);
}

// Java: private DataComponentPatch asPatch()
LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_PatchedDataComponentMap_AsPatch(
        const LIBMATTI_MC_PatchedDataComponentMap *map)
{
    LIBMATTI_MC_DataComponentPatch *patch = LIBMATTI_MC_DataComponentPatch_New();
    for (size_t i = 0; i < map->patchSize; i++)
    {
        if (map->patchPresent[i])
            LIBMATTI_MC_DataComponentPatch_Set(patch, map->patchTypes[i], map->patchValues[i]);
        else
            LIBMATTI_MC_DataComponentPatch_Remove(patch, map->patchTypes[i]);
    }
    return patch;
}

// Java: public PatchedDataComponentMap copy()
LIBMATTI_MC_PatchedDataComponentMap *LIBMATTI_MC_PatchedDataComponentMap_Copy(
        const LIBMATTI_MC_PatchedDataComponentMap *map)
{
    LIBMATTI_MC_PatchedDataComponentMap *copy = LIBMATTI_MC_PatchedDataComponentMap_New(map->prototype);
    for (size_t i = 0; i < map->patchSize; i++)
        patch_entry_put(copy, map->patchTypes[i], map->patchPresent[i], map->patchValues[i]);
    return copy;
}

// Java: public Set<DataComponentType<?>> keySet() - prototype keys minus removed plus added
LIBMATTI_MC_DataComponentType **LIBMATTI_MC_PatchedDataComponentMap_KeySet(
        const LIBMATTI_MC_PatchedDataComponentMap *map, size_t *count)
{
    size_t prototypeCount = 0;
    LIBMATTI_MC_DataComponentType **prototypeKeys =
            LIBMATTI_MC_DataComponentMap_KeySet(map->prototype, &prototypeCount);
    size_t total = prototypeCount + map->patchSize;
    LIBMATTI_MC_DataComponentType **keys = malloc(sizeof(LIBMATTI_MC_DataComponentType *) * (total > 0 ? total : 1));
    size_t keyCount = 0;
    for (size_t i = 0; i < prototypeCount; i++)
    {
        if (patch_index(map, prototypeKeys[i]) < 0)
            keys[keyCount++] = prototypeKeys[i];
    }
    for (size_t i = 0; i < map->patchSize; i++)
    {
        if (map->patchPresent[i])
            keys[keyCount++] = map->patchTypes[i];
    }
    if (count != NULL)
        *count = keyCount;
    return keys;
}

void LIBMATTI_MC_PatchedDataComponentMap_Free(LIBMATTI_MC_PatchedDataComponentMap *map)
{
    if (map == NULL)
        return;
    free(map->patchTypes);
    free(map->patchPresent);
    free(map->patchValues);
    free(map);
}

// Java: PatchedDataComponentMap equals - patch and prototype must resolve to the same values
bool LIBMATTI_MC_PatchedDataComponentMap_Equals(const LIBMATTI_MC_PatchedDataComponentMap *a,
                                                const LIBMATTI_MC_PatchedDataComponentMap *b)
{
    if (a == b)
        return true;
    if (a == NULL || b == NULL)
        return false;
    size_t countA = 0;
    LIBMATTI_MC_DataComponentType **keysA = LIBMATTI_MC_PatchedDataComponentMap_KeySet(a, &countA);
    size_t countB = 0;
    LIBMATTI_MC_DataComponentType **keysB = LIBMATTI_MC_PatchedDataComponentMap_KeySet(b, &countB);
    bool equal = countA == countB;
    if (equal)
    {
        for (size_t i = 0; i < countA && equal; i++)
        {
            void *valueA = LIBMATTI_MC_PatchedDataComponentMap_Get(a, keysA[i]);
            void *valueB = LIBMATTI_MC_PatchedDataComponentMap_Get(b, keysA[i]);
            if (valueA != valueB)
                equal = false;
        }
    }
    free(keysA);
    free(keysB);
    return equal;
}
