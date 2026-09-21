// Port of java.util.HashMap<K,V> for the K=String case the codebase uses
// (module names, package names, config names, ...). Keys are copied on put
// and owned by the map; values stay owned by the caller.

#ifndef MATTICRAFT_JAVA_UTIL_HASHMAP_H
#define MATTICRAFT_JAVA_UTIL_HASHMAP_H

#include <stddef.h>

typedef struct LIBMATTI_JU_HashMapEntry
{
    char *key;
    void *value;
    size_t hash;
    struct LIBMATTI_JU_HashMapEntry *next;
} LIBMATTI_JU_HashMapEntry;

typedef struct LIBMATTI_JU_HashMap
{
    LIBMATTI_JU_HashMapEntry **table;
    size_t capacity; // Java: table.length, always a power of two
    size_t size;
} LIBMATTI_JU_HashMap;

// Java: public HashMap()
LIBMATTI_JU_HashMap *LIBMATTI_JU_HashMap_New(void);
// Java: public HashMap(int initialCapacity)
LIBMATTI_JU_HashMap *LIBMATTI_JU_HashMap_NewWithCapacity(size_t initialCapacity);
void LIBMATTI_JU_HashMap_Free(LIBMATTI_JU_HashMap *map);

// Java: public int size()
size_t LIBMATTI_JU_HashMap_Size(const LIBMATTI_JU_HashMap *map);
// Java: public boolean isEmpty()
int LIBMATTI_JU_HashMap_IsEmpty(const LIBMATTI_JU_HashMap *map);

// Java: public V get(Object key) - NULL when absent (Java null)
void *LIBMATTI_JU_HashMap_Get(const LIBMATTI_JU_HashMap *map, const char *key);
// Java: public V getOrDefault(Object key, V defaultValue)
void *LIBMATTI_JU_HashMap_GetOrDefault(const LIBMATTI_JU_HashMap *map, const char *key, void *defaultValue);
// Java: public boolean containsKey(Object key)
int LIBMATTI_JU_HashMap_ContainsKey(const LIBMATTI_JU_HashMap *map, const char *key);
// Java: public V put(K key, V value) - returns the old value (NULL = none)
void *LIBMATTI_JU_HashMap_Put(LIBMATTI_JU_HashMap *map, const char *key, void *value);
// Java: public V putIfAbsent(K key, V value)
void *LIBMATTI_JU_HashMap_PutIfAbsent(LIBMATTI_JU_HashMap *map, const char *key, void *value);
// Java: public V computeIfAbsent(K key, Function<K,V> mappingFunction) - the
// function receives the key and self; returning NULL stores nothing
void *LIBMATTI_JU_HashMap_ComputeIfAbsent(LIBMATTI_JU_HashMap *map, const char *key,
                                          void *(*function)(const char *key, void *self), void *self);
// Java: public V remove(Object key) - returns the removed value
void *LIBMATTI_JU_HashMap_Remove(LIBMATTI_JU_HashMap *map, const char *key);
// Java: public void clear()
void LIBMATTI_JU_HashMap_Clear(LIBMATTI_JU_HashMap *map);

// Java: public void forEach(BiConsumer<K,V>) - the callback gets key and value
void LIBMATTI_JU_HashMap_ForEach(const LIBMATTI_JU_HashMap *map,
                                 void (*action)(const char *key, void *value, void *self), void *self);

// The keys point into the map's entries; valid until the next modification.
char **LIBMATTI_JU_HashMap_KeySet(const LIBMATTI_JU_HashMap *map, size_t *count);

#endif //MATTICRAFT_JAVA_UTIL_HASHMAP_H
