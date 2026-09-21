#include "libmatti/java/util/HashSet.h"

#include "libmatti/java/util/HashMap.h"

#include <stdlib.h>

// The set is a HashMap<E,Object> with a shared dummy value, exactly like Java's impl.
static int PRESENT_DUMMY;

struct LIBMATTI_JU_HashSet
{
    LIBMATTI_JU_HashMap *map;
};

LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_New(void)
{
    LIBMATTI_JU_HashSet *set = calloc(1, sizeof(LIBMATTI_JU_HashSet));
    set->map = LIBMATTI_JU_HashMap_New();
    return set;
}

LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_NewFrom(const char *const *items, size_t count)
{
    LIBMATTI_JU_HashSet *set = LIBMATTI_JU_HashSet_New();
    for (size_t i = 0; i < count; i++)
        LIBMATTI_JU_HashSet_Add(set, items[i]);
    return set;
}

LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_CopyOf(const char *const *items, size_t count)
{
    return LIBMATTI_JU_HashSet_NewFrom(items, count);
}

void LIBMATTI_JU_HashSet_Free(LIBMATTI_JU_HashSet *set)
{
    if (set == NULL)
        return;
    LIBMATTI_JU_HashMap_Free(set->map);
    free(set);
}

size_t LIBMATTI_JU_HashSet_Size(const LIBMATTI_JU_HashSet *set)
{
    return LIBMATTI_JU_HashMap_Size(set->map);
}

int LIBMATTI_JU_HashSet_IsEmpty(const LIBMATTI_JU_HashSet *set)
{
    return LIBMATTI_JU_HashMap_IsEmpty(set->map);
}

int LIBMATTI_JU_HashSet_Add(LIBMATTI_JU_HashSet *set, const char *item)
{
    // Java: map.put(e, PRESENT) == null
    return LIBMATTI_JU_HashMap_Put(set->map, item, &PRESENT_DUMMY) == NULL;
}

int LIBMATTI_JU_HashSet_Contains(const LIBMATTI_JU_HashSet *set, const char *item)
{
    return LIBMATTI_JU_HashMap_ContainsKey(set->map, item);
}

int LIBMATTI_JU_HashSet_Remove(LIBMATTI_JU_HashSet *set, const char *item)
{
    return LIBMATTI_JU_HashMap_Remove(set->map, item) != NULL;
}

void LIBMATTI_JU_HashSet_Clear(LIBMATTI_JU_HashSet *set)
{
    LIBMATTI_JU_HashMap_Clear(set->map);
}

void LIBMATTI_JU_HashSet_ForEach(const LIBMATTI_JU_HashSet *set,
                                 void (*action)(const char *item, void *self), void *self)
{
    LIBMATTI_JU_HashMap_ForEach(set->map, (void (*)(const char *, void *, void *)) action, self);
}

char **LIBMATTI_JU_HashSet_ToArray(const LIBMATTI_JU_HashSet *set, size_t *count)
{
    return LIBMATTI_JU_HashMap_KeySet(set->map, count);
}
