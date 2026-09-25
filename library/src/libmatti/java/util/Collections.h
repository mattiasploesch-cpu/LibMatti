// Port of the java.util.Collections unmodifiable wrappers. Java returns
// delegate views whose mutators throw UnsupportedOperationException; the C
// port models the view as a tagged wrapper over the backing collection and
// the mutators log + abort (the strict Java semantics, minus the exception
// unwind) so call sites catch violations immediately.

#ifndef MATTICRAFT_JAVA_UTIL_COLLECTIONS_H
#define MATTICRAFT_JAVA_UTIL_COLLECTIONS_H

#include "libmatti/java/util/ArrayList.h"
#include "libmatti/java/util/HashMap.h"
#include "libmatti/java/util/HashSet.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: Collections.unmodifiableList(List) - a read-only view; the C port
// wraps the same LIBMATTI_JU_ArrayList pointer (the view struct exists so the
// mutator paths can be intercepted).
typedef struct LIBMATTI_JU_UnmodifiableList
{
    const LIBMATTI_JU_ArrayList *backing;
} LIBMATTI_JU_UnmodifiableList;

// Java: Collections.unmodifiableMap(Map)
typedef struct LIBMATTI_JU_UnmodifiableMap
{
    const LIBMATTI_JU_HashMap *backing;
} LIBMATTI_JU_UnmodifiableMap;

// Java: Collections.unmodifiableSet(Set)
typedef struct LIBMATTI_JU_UnmodifiableSet
{
    const LIBMATTI_JU_HashSet *backing;
} LIBMATTI_JU_UnmodifiableSet;

// Java: Collections.unmodifiableList(list) - NULL on NULL
LIBMATTI_JU_UnmodifiableList *LIBMATTI_JU_Collections_UnmodifiableList(const LIBMATTI_JU_ArrayList *list);
LIBMATTI_JU_UnmodifiableMap *LIBMATTI_JU_Collections_UnmodifiableMap(const LIBMATTI_JU_HashMap *map);
LIBMATTI_JU_UnmodifiableSet *LIBMATTI_JU_Collections_UnmodifiableSet(const LIBMATTI_JU_HashSet *set);

// Read passthrough - Java: size()/isEmpty()/get(int)/contains(Object)
size_t LIBMATTI_JU_UnmodifiableList_Size(const LIBMATTI_JU_UnmodifiableList *list);
int LIBMATTI_JU_UnmodifiableList_IsEmpty(const LIBMATTI_JU_UnmodifiableList *list);
void *LIBMATTI_JU_UnmodifiableList_Get(const LIBMATTI_JU_UnmodifiableList *list, size_t index);
int LIBMATTI_JU_UnmodifiableList_Contains(const LIBMATTI_JU_UnmodifiableList *list, const void *element);

// Java: size()/isEmpty()/get(Object)/containsKey(String)
size_t LIBMATTI_JU_UnmodifiableMap_Size(const LIBMATTI_JU_UnmodifiableMap *map);
int LIBMATTI_JU_UnmodifiableMap_IsEmpty(const LIBMATTI_JU_UnmodifiableMap *map);
void *LIBMATTI_JU_UnmodifiableMap_Get(const LIBMATTI_JU_UnmodifiableMap *map, const char *key);
int LIBMATTI_JU_UnmodifiableMap_ContainsKey(const LIBMATTI_JU_UnmodifiableMap *map, const char *key);

// Java: size()/isEmpty()/contains(Object) - the port's HashSet is string-keyed
size_t LIBMATTI_JU_UnmodifiableSet_Size(const LIBMATTI_JU_UnmodifiableSet *set);
int LIBMATTI_JU_UnmodifiableSet_IsEmpty(const LIBMATTI_JU_UnmodifiableSet *set);
int LIBMATTI_JU_UnmodifiableSet_Contains(const LIBMATTI_JU_UnmodifiableSet *set, const char *item);

// Java: every mutator throws UnsupportedOperationException - the port logs
// "UnsupportedOperationException" and aborts (fail-fast like the exception).
void LIBMATTI_JU_UnmodifiableList_Add(LIBMATTI_JU_UnmodifiableList *list, void *element);
void LIBMATTI_JU_UnmodifiableList_Set(LIBMATTI_JU_UnmodifiableList *list, size_t index, void *element);
void LIBMATTI_JU_UnmodifiableList_RemoveAt(LIBMATTI_JU_UnmodifiableList *list, size_t index);
void LIBMATTI_JU_UnmodifiableMap_Put(LIBMATTI_JU_UnmodifiableMap *map, const char *key, void *value);
void LIBMATTI_JU_UnmodifiableMap_Remove(LIBMATTI_JU_UnmodifiableMap *map, const char *key);
void LIBMATTI_JU_UnmodifiableSet_Add(LIBMATTI_JU_UnmodifiableSet *set, const char *item);
void LIBMATTI_JU_UnmodifiableSet_Remove(LIBMATTI_JU_UnmodifiableSet *set, const char *item);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_JAVA_UTIL_COLLECTIONS_H
