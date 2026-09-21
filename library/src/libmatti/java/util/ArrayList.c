#include "libmatti/java/util/ArrayList.h"

#include <stdlib.h>
#include <string.h>

static void ensure_capacity(LIBMATTI_JU_ArrayList *list, size_t needed)
{
    // Java: grow(int minCapacity) - newCapacity = old + (old >> 1), at least the needed size
    if (needed <= list->capacity)
        return;
    size_t newCapacity = list->capacity + (list->capacity >> 1);
    if (newCapacity < needed)
        newCapacity = needed;
    if (newCapacity < 10)
        newCapacity = 10; // Java: DEFAULT_CAPACITY
    list->elements = realloc(list->elements, sizeof(void *) * newCapacity);
    list->capacity = newCapacity;
}

LIBMATTI_JU_ArrayList *LIBMATTI_JU_ArrayList_New(void)
{
    return LIBMATTI_JU_ArrayList_NewWithCapacity(10);
}

LIBMATTI_JU_ArrayList *LIBMATTI_JU_ArrayList_NewWithCapacity(size_t initialCapacity)
{
    LIBMATTI_JU_ArrayList *list = calloc(1, sizeof(LIBMATTI_JU_ArrayList));
    if (initialCapacity > 0)
    {
        list->elements = malloc(sizeof(void *) * initialCapacity);
        list->capacity = initialCapacity;
    }
    return list;
}

void LIBMATTI_JU_ArrayList_Free(LIBMATTI_JU_ArrayList *list)
{
    if (list == NULL)
        return;
    free(list->elements);
    free(list);
}

size_t LIBMATTI_JU_ArrayList_Size(const LIBMATTI_JU_ArrayList *list)
{
    return list->size;
}

int LIBMATTI_JU_ArrayList_IsEmpty(const LIBMATTI_JU_ArrayList *list)
{
    return list->size == 0;
}

int LIBMATTI_JU_ArrayList_Add(LIBMATTI_JU_ArrayList *list, void *element)
{
    ensure_capacity(list, list->size + 1);
    if (list->elements == NULL)
        return 0;
    list->elements[list->size++] = element;
    return 1;
}

int LIBMATTI_JU_ArrayList_AddAt(LIBMATTI_JU_ArrayList *list, size_t index, void *element)
{
    if (index > list->size)
        return 0;
    ensure_capacity(list, list->size + 1);
    if (list->elements == NULL)
        return 0;
    // Java: System.arraycopy(elementData, index, elementData, index + 1, size - index)
    memmove(list->elements + index + 1, list->elements + index, sizeof(void *) * (list->size - index));
    list->elements[index] = element;
    list->size++;
    return 1;
}

void *LIBMATTI_JU_ArrayList_Get(const LIBMATTI_JU_ArrayList *list, size_t index)
{
    // Java: Objects.checkIndex -> IndexOutOfBoundsException
    if (index >= list->size)
        return NULL;
    return list->elements[index];
}

void *LIBMATTI_JU_ArrayList_Set(LIBMATTI_JU_ArrayList *list, size_t index, void *element)
{
    if (index >= list->size)
        return NULL;
    void *old = list->elements[index];
    list->elements[index] = element;
    return old;
}

void *LIBMATTI_JU_ArrayList_RemoveAt(LIBMATTI_JU_ArrayList *list, size_t index)
{
    if (index >= list->size)
        return NULL;
    void *removed = list->elements[index];
    // Java: System.arraycopy(elementData, index + 1, elementData, index, size - index - 1)
    memmove(list->elements + index, list->elements + index + 1, sizeof(void *) * (list->size - index - 1));
    list->size--;
    return removed;
}

int LIBMATTI_JU_ArrayList_RemoveByPointer(LIBMATTI_JU_ArrayList *list, const void *element)
{
    long index = LIBMATTI_JU_ArrayList_IndexOf(list, element);
    if (index < 0)
        return 0;
    LIBMATTI_JU_ArrayList_RemoveAt(list, (size_t) index);
    return 1;
}

int LIBMATTI_JU_ArrayList_Contains(const LIBMATTI_JU_ArrayList *list, const void *element)
{
    return LIBMATTI_JU_ArrayList_IndexOf(list, element) >= 0;
}

long LIBMATTI_JU_ArrayList_IndexOf(const LIBMATTI_JU_ArrayList *list, const void *element)
{
    // Java: indexOf - null-safe equality; the port compares pointers (default equals)
    for (size_t i = 0; i < list->size; i++)
        if (list->elements[i] == element)
            return (long) i;
    return -1;
}

void LIBMATTI_JU_ArrayList_Clear(LIBMATTI_JU_ArrayList *list)
{
    list->size = 0;
}

void LIBMATTI_JU_ArrayList_EnsureCapacity(LIBMATTI_JU_ArrayList *list, size_t minCapacity)
{
    ensure_capacity(list, minCapacity);
}

void LIBMATTI_JU_ArrayList_Sort(LIBMATTI_JU_ArrayList *list, int (*comparator)(const void *a, const void *b))
{
    // Java: Arrays.sort -> TimSort; qsort is the C equivalent
    if (list->size > 1 && comparator != NULL)
        qsort(list->elements, list->size, sizeof(void *), comparator);
}
