// Port of java.util.ArrayList<E>.
// E is void* in the C port; the list does not own the elements (the owner frees).

#ifndef MATTICRAFT_JAVA_UTIL_ARRAYLIST_H
#define MATTICRAFT_JAVA_UTIL_ARRAYLIST_H

#include <stddef.h>

typedef struct LIBMATTI_JU_ArrayList
{
    void **elements;
    size_t size;
    size_t capacity;
} LIBMATTI_JU_ArrayList;

// Java: public ArrayList()
LIBMATTI_JU_ArrayList *LIBMATTI_JU_ArrayList_New(void);
// Java: public ArrayList(int initialCapacity)
LIBMATTI_JU_ArrayList *LIBMATTI_JU_ArrayList_NewWithCapacity(size_t initialCapacity);
void LIBMATTI_JU_ArrayList_Free(LIBMATTI_JU_ArrayList *list);

// Java: public int size()
size_t LIBMATTI_JU_ArrayList_Size(const LIBMATTI_JU_ArrayList *list);
// Java: public boolean isEmpty()
int LIBMATTI_JU_ArrayList_IsEmpty(const LIBMATTI_JU_ArrayList *list);

// Java: public boolean add(E e) - 0 on allocation failure
int LIBMATTI_JU_ArrayList_Add(LIBMATTI_JU_ArrayList *list, void *element);
// Java: public void add(int index, E element) - 0 on failure (index out of range / OOM)
int LIBMATTI_JU_ArrayList_AddAt(LIBMATTI_JU_ArrayList *list, size_t index, void *element);
// Java: public E get(int index) - NULL on out of range
void *LIBMATTI_JU_ArrayList_Get(const LIBMATTI_JU_ArrayList *list, size_t index);
// Java: public E set(int index, E element) - returns the old element, NULL on out of range
void *LIBMATTI_JU_ArrayList_Set(LIBMATTI_JU_ArrayList *list, size_t index, void *element);
// Java: public E remove(int index) - returns the removed element, NULL on out of range
void *LIBMATTI_JU_ArrayList_RemoveAt(LIBMATTI_JU_ArrayList *list, size_t index);
// Java: public boolean remove(Object o) - removes the first element identical (==) to o
int LIBMATTI_JU_ArrayList_RemoveByPointer(LIBMATTI_JU_ArrayList *list, const void *element);
// Java: public boolean contains(Object o) - pointer identity, like the default equals
int LIBMATTI_JU_ArrayList_Contains(const LIBMATTI_JU_ArrayList *list, const void *element);
// Java: public int indexOf(Object o) - -1 when absent
long LIBMATTI_JU_ArrayList_IndexOf(const LIBMATTI_JU_ArrayList *list, const void *element);
// Java: public void clear()
void LIBMATTI_JU_ArrayList_Clear(LIBMATTI_JU_ArrayList *list);

// Java: private void grow(int minCapacity) - public here because the loader pre-sizes lists
void LIBMATTI_JU_ArrayList_EnsureCapacity(LIBMATTI_JU_ArrayList *list, size_t minCapacity);

// Java: Collections.sort(list, comparator)
void LIBMATTI_JU_ArrayList_Sort(LIBMATTI_JU_ArrayList *list, int (*comparator)(const void *a, const void *b));

#endif //MATTICRAFT_JAVA_UTIL_ARRAYLIST_H
