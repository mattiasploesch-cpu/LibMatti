#include "libmatti/java/util/Comparator.h"

#include "libmatti/java/util/ArrayList.h"

#include <stdlib.h>
#include <string.h>

int LIBMATTI_JU_Comparator_StringCompare(const void *a, const void *b)
{
    return strcmp((const char *) a, (const char *) b);
}

int LIBMATTI_JU_Comparator_StringCompareIgnoreCase(const void *a, const void *b)
{
    const unsigned char *s1 = (const unsigned char *) a;
    const unsigned char *s2 = (const unsigned char *) b;
    while (*s1 != '\0' && *s2 != '\0')
    {
        int c1 = *s1 >= 'A' && *s1 <= 'Z' ? *s1 + 32 : *s1;
        int c2 = *s2 >= 'A' && *s2 <= 'Z' ? *s2 + 32 : *s2;
        if (c1 != c2)
            return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int LIBMATTI_JU_Comparator_StringReverse(const void *a, const void *b)
{
    return -LIBMATTI_JU_Comparator_StringCompare(a, b);
}

int LIBMATTI_JU_Comparator_SizeTBy(const void *a, const void *b)
{
    return *(const size_t *) a < *(const size_t *) b ? -1 : (*(const size_t *) a > *(const size_t *) b ? 1 : 0);
}

LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_Reverse(LIBMATTI_JU_Comparator comparator)
{
    LIBMATTI_JU_ComparatorHolder *holder = malloc(sizeof(LIBMATTI_JU_ComparatorHolder));
    holder->wrapped = comparator;
    return holder;
}

int LIBMATTI_JU_Comparator_ReverseFn(const void *a, const void *b)
{
    const LIBMATTI_JU_ComparatorHolder *holder = a;
    return -holder->wrapped(b, a);
}

LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_NullsFirst(LIBMATTI_JU_Comparator comparator)
{
    LIBMATTI_JU_ComparatorHolder *holder = malloc(sizeof(LIBMATTI_JU_ComparatorHolder));
    holder->wrapped = comparator;
    return holder;
}

LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_NullsLast(LIBMATTI_JU_Comparator comparator)
{
    LIBMATTI_JU_ComparatorHolder *holder = malloc(sizeof(LIBMATTI_JU_ComparatorHolder));
    holder->wrapped = comparator;
    return holder;
}

int LIBMATTI_JU_Comparator_NullsFirstFn(const void *a, const void *b)
{
    const LIBMATTI_JU_ComparatorHolder *holder = a;
    if (b == NULL)
        return a == NULL ? 0 : 1;
    if (a == NULL)
        return -1;
    return holder->wrapped(a, b);
}

int LIBMATTI_JU_Comparator_NullsLastFn(const void *a, const void *b)
{
    const LIBMATTI_JU_ComparatorHolder *holder = a;
    if (a == NULL)
        return b == NULL ? 0 : 1;
    if (b == NULL)
        return -1;
    return holder->wrapped(a, b);
}

void LIBMATTI_JU_Collections_Swap(struct LIBMATTI_JU_ArrayList *list, size_t i, size_t j)
{
    void *tmp = list->elements[i];
    list->elements[i] = list->elements[j];
    list->elements[j] = tmp;
}

void LIBMATTI_JU_Collections_Reverse(struct LIBMATTI_JU_ArrayList *list)
{
    // Java: Collections.reverse
    if (list->size < 2)
        return;
    for (size_t i = 0, j = list->size - 1; i < j; i++, j--)
        LIBMATTI_JU_Collections_Swap(list, i, j);
}

void LIBMATTI_JU_Collections_Sort(struct LIBMATTI_JU_ArrayList *list, LIBMATTI_JU_Comparator comparator)
{
    LIBMATTI_JU_ArrayList_Sort(list, comparator);
}
