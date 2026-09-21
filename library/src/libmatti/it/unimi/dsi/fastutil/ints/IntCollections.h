// Port of the it.unimi.dsi.fastutil.ints surface the game uses:
// Int2ObjectMap, IntOpenHashSet, IntArrayList, IntObjectImmutablePair.

#ifndef MATTICRAFT_FASTUTIL_INTS_H
#define MATTICRAFT_FASTUTIL_INTS_H

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Java: it.unimi.dsi.fastutil.ints.Int2ObjectOpenHashMap<V>
// ---------------------------------------------------------------------------

typedef struct
{
    int32_t key;
    void *value; // owned by the caller (like the Java map not owning values)
    int used;
} LIBMATTI_FU_Int2ObjectEntry;

typedef struct
{
    LIBMATTI_FU_Int2ObjectEntry *entries;
    size_t capacity; // power of two
    size_t size;
} LIBMATTI_FU_Int2ObjectMap;

LIBMATTI_FU_Int2ObjectMap *LIBMATTI_FU_Int2ObjectMap_New(void);
void LIBMATTI_FU_Int2ObjectMap_Free(LIBMATTI_FU_Int2ObjectMap *map);

// Java: public V put(final int key, final V value) - returns the old value
void *LIBMATTI_FU_Int2ObjectMap_Put(LIBMATTI_FU_Int2ObjectMap *map, int32_t key, void *value);
// Java: public V get(final Object key) - NULL when absent
void *LIBMATTI_FU_Int2ObjectMap_Get(const LIBMATTI_FU_Int2ObjectMap *map, int32_t key);
// Java: public V remove(final int key)
void *LIBMATTI_FU_Int2ObjectMap_Remove(LIBMATTI_FU_Int2ObjectMap *map, int32_t key);
// Java: public boolean containsKey(final int key)
int LIBMATTI_FU_Int2ObjectMap_ContainsKey(const LIBMATTI_FU_Int2ObjectMap *map, int32_t key);
// Java: public int size()
size_t LIBMATTI_FU_Int2ObjectMap_Size(const LIBMATTI_FU_Int2ObjectMap *map);
// Java: public void forEach(BiConsumer<Integer, V>)
void LIBMATTI_FU_Int2ObjectMap_ForEach(const LIBMATTI_FU_Int2ObjectMap *map,
                                       void (*action)(int32_t key, void *value, void *self), void *self);

// ---------------------------------------------------------------------------
// Java: it.unimi.dsi.fastutil.ints.IntOpenHashSet
// ---------------------------------------------------------------------------

typedef struct
{
    int32_t *keys;
    unsigned char *used;
    size_t capacity; // power of two
    size_t size;
} LIBMATTI_FU_IntOpenHashSet;

LIBMATTI_FU_IntOpenHashSet *LIBMATTI_FU_IntOpenHashSet_New(void);
void LIBMATTI_FU_IntOpenHashSet_Free(LIBMATTI_FU_IntOpenHashSet *set);
// Java: public boolean add(final int k) - 1 when newly added
int LIBMATTI_FU_IntOpenHashSet_Add(LIBMATTI_FU_IntOpenHashSet *set, int32_t key);
// Java: public boolean contains(final int k)
int LIBMATTI_FU_IntOpenHashSet_Contains(const LIBMATTI_FU_IntOpenHashSet *set, int32_t key);
// Java: public boolean remove(final int k)
int LIBMATTI_FU_IntOpenHashSet_Remove(LIBMATTI_FU_IntOpenHashSet *set, int32_t key);
size_t LIBMATTI_FU_IntOpenHashSet_Size(const LIBMATTI_FU_IntOpenHashSet *set);

// ---------------------------------------------------------------------------
// Java: it.unimi.dsi.fastutil.ints.IntArrayList
// ---------------------------------------------------------------------------

typedef struct
{
    int32_t *elements;
    size_t size;
    size_t capacity;
} LIBMATTI_FU_IntArrayList;

LIBMATTI_FU_IntArrayList *LIBMATTI_FU_IntArrayList_New(void);
void LIBMATTI_FU_IntArrayList_Free(LIBMATTI_FU_IntArrayList *list);
void LIBMATTI_FU_IntArrayList_Add(LIBMATTI_FU_IntArrayList *list, int32_t value);
int32_t LIBMATTI_FU_IntArrayList_Get(const LIBMATTI_FU_IntArrayList *list, size_t index);
size_t LIBMATTI_FU_IntArrayList_Size(const LIBMATTI_FU_IntArrayList *list);
// Java: public int[] toIntArray()
int32_t *LIBMATTI_FU_IntArrayList_ToArray(const LIBMATTI_FU_IntArrayList *list, size_t *count);

#endif //MATTICRAFT_FASTUTIL_INTS_H
