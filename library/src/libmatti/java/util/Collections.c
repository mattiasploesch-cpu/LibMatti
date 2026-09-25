// Port of the java.util.Collections unmodifiable wrappers (implementation).

#include "libmatti/java/util/Collections.h"

#include <stdio.h>
#include <stdlib.h>

// Java: the mutators throw UnsupportedOperationException; the port logs the
// exception name and aborts - fail-fast without the unwinding.
static void unsupported(const char *method)
{
    fprintf(stderr, "java.lang.UnsupportedOperationException: %s on unmodifiable view\n", method);
    abort();
}

LIBMATTI_JU_UnmodifiableList *LIBMATTI_JU_Collections_UnmodifiableList(const LIBMATTI_JU_ArrayList *list)
{
    if (list == NULL)
        return NULL;
    LIBMATTI_JU_UnmodifiableList *view = malloc(sizeof(LIBMATTI_JU_UnmodifiableList));
    if (view != NULL)
        view->backing = list;
    return view;
}

LIBMATTI_JU_UnmodifiableMap *LIBMATTI_JU_Collections_UnmodifiableMap(const LIBMATTI_JU_HashMap *map)
{
    if (map == NULL)
        return NULL;
    LIBMATTI_JU_UnmodifiableMap *view = malloc(sizeof(LIBMATTI_JU_UnmodifiableMap));
    if (view != NULL)
        view->backing = map;
    return view;
}

LIBMATTI_JU_UnmodifiableSet *LIBMATTI_JU_Collections_UnmodifiableSet(const LIBMATTI_JU_HashSet *set)
{
    if (set == NULL)
        return NULL;
    LIBMATTI_JU_UnmodifiableSet *view = malloc(sizeof(LIBMATTI_JU_UnmodifiableSet));
    if (view != NULL)
        view->backing = set;
    return view;
}

size_t LIBMATTI_JU_UnmodifiableList_Size(const LIBMATTI_JU_UnmodifiableList *list)
{
    return list != NULL ? LIBMATTI_JU_ArrayList_Size(list->backing) : 0;
}

int LIBMATTI_JU_UnmodifiableList_IsEmpty(const LIBMATTI_JU_UnmodifiableList *list)
{
    return list != NULL ? LIBMATTI_JU_ArrayList_IsEmpty(list->backing) : 1;
}

void *LIBMATTI_JU_UnmodifiableList_Get(const LIBMATTI_JU_UnmodifiableList *list, size_t index)
{
    return list != NULL ? LIBMATTI_JU_ArrayList_Get(list->backing, index) : NULL;
}

int LIBMATTI_JU_UnmodifiableList_Contains(const LIBMATTI_JU_UnmodifiableList *list, const void *element)
{
    return list != NULL ? LIBMATTI_JU_ArrayList_Contains(list->backing, element) : 0;
}

size_t LIBMATTI_JU_UnmodifiableMap_Size(const LIBMATTI_JU_UnmodifiableMap *map)
{
    return map != NULL ? LIBMATTI_JU_HashMap_Size(map->backing) : 0;
}

int LIBMATTI_JU_UnmodifiableMap_IsEmpty(const LIBMATTI_JU_UnmodifiableMap *map)
{
    return map != NULL ? LIBMATTI_JU_HashMap_IsEmpty(map->backing) : 1;
}

void *LIBMATTI_JU_UnmodifiableMap_Get(const LIBMATTI_JU_UnmodifiableMap *map, const char *key)
{
    return map != NULL ? LIBMATTI_JU_HashMap_Get(map->backing, key) : NULL;
}

int LIBMATTI_JU_UnmodifiableMap_ContainsKey(const LIBMATTI_JU_UnmodifiableMap *map, const char *key)
{
    return map != NULL ? LIBMATTI_JU_HashMap_ContainsKey(map->backing, key) : 0;
}

size_t LIBMATTI_JU_UnmodifiableSet_Size(const LIBMATTI_JU_UnmodifiableSet *set)
{
    return set != NULL ? LIBMATTI_JU_HashSet_Size(set->backing) : 0;
}

int LIBMATTI_JU_UnmodifiableSet_IsEmpty(const LIBMATTI_JU_UnmodifiableSet *set)
{
    return set != NULL ? LIBMATTI_JU_HashSet_IsEmpty(set->backing) : 1;
}

int LIBMATTI_JU_UnmodifiableSet_Contains(const LIBMATTI_JU_UnmodifiableSet *set, const char *item)
{
    return set != NULL ? LIBMATTI_JU_HashSet_Contains(set->backing, item) : 0;
}

void LIBMATTI_JU_UnmodifiableList_Add(LIBMATTI_JU_UnmodifiableList *list, void *element)
{
    (void) list;
    (void) element;
    unsupported("add");
}

void LIBMATTI_JU_UnmodifiableList_Set(LIBMATTI_JU_UnmodifiableList *list, size_t index, void *element)
{
    (void) list;
    (void) index;
    (void) element;
    unsupported("set");
}

void LIBMATTI_JU_UnmodifiableList_RemoveAt(LIBMATTI_JU_UnmodifiableList *list, size_t index)
{
    (void) list;
    (void) index;
    unsupported("remove");
}

void LIBMATTI_JU_UnmodifiableMap_Put(LIBMATTI_JU_UnmodifiableMap *map, const char *key, void *value)
{
    (void) map;
    (void) key;
    (void) value;
    unsupported("put");
}

void LIBMATTI_JU_UnmodifiableMap_Remove(LIBMATTI_JU_UnmodifiableMap *map, const char *key)
{
    (void) map;
    (void) key;
    unsupported("remove");
}

void LIBMATTI_JU_UnmodifiableSet_Add(LIBMATTI_JU_UnmodifiableSet *set, const char *item)
{
    (void) set;
    (void) item;
    unsupported("add");
}

void LIBMATTI_JU_UnmodifiableSet_Remove(LIBMATTI_JU_UnmodifiableSet *set, const char *item)
{
    (void) set;
    (void) item;
    unsupported("remove");
}
