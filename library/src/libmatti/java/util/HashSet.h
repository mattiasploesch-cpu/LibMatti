// Port of java.util.HashSet<String> (the set type the codebase actually uses:
// package sets, module name sets, config sets). Strings are copied on add and
// owned by the set.

#ifndef MATTICRAFT_JAVA_UTIL_HASHSET_H
#define MATTICRAFT_JAVA_UTIL_HASHSET_H

#include <stddef.h>

typedef struct LIBMATTI_JU_HashSet LIBMATTI_JU_HashSet;

// Java: public HashSet()
LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_New(void);
// Java: public HashSet(Collection<? extends E> c)
LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_NewFrom(const char *const *items, size_t count);
void LIBMATTI_JU_HashSet_Free(LIBMATTI_JU_HashSet *set);

// Java: public int size()
size_t LIBMATTI_JU_HashSet_Size(const LIBMATTI_JU_HashSet *set);
// Java: public boolean isEmpty()
int LIBMATTI_JU_HashSet_IsEmpty(const LIBMATTI_JU_HashSet *set);

// Java: public boolean add(E e) - copies the string; 1 when newly added
int LIBMATTI_JU_HashSet_Add(LIBMATTI_JU_HashSet *set, const char *item);
// Java: public boolean contains(Object o)
int LIBMATTI_JU_HashSet_Contains(const LIBMATTI_JU_HashSet *set, const char *item);
// Java: public boolean remove(Object o)
int LIBMATTI_JU_HashSet_Remove(LIBMATTI_JU_HashSet *set, const char *item);
// Java: public void clear()
void LIBMATTI_JU_HashSet_Clear(LIBMATTI_JU_HashSet *set);

// Java: public void forEach(Consumer<E>) - the callback gets the set-owned string
void LIBMATTI_JU_HashSet_ForEach(const LIBMATTI_JU_HashSet *set,
                                 void (*action)(const char *item, void *self), void *self);

// Java: public Set.copyOf(c) - copies the strings into a new set
LIBMATTI_JU_HashSet *LIBMATTI_JU_HashSet_CopyOf(const char *const *items, size_t count);

// The strings are set-owned; valid until the set is modified or freed.
char **LIBMATTI_JU_HashSet_ToArray(const LIBMATTI_JU_HashSet *set, size_t *count);

#endif //MATTICRAFT_JAVA_UTIL_HASHSET_H
