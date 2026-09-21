// Port of java.util.Comparator<T> and the java.util.Collections helpers.

#ifndef MATTICRAFT_JAVA_UTIL_COMPARATOR_H
#define MATTICRAFT_JAVA_UTIL_COMPARATOR_H

#include <stddef.h>

struct LIBMATTI_JU_ArrayList;

// Java: interface Comparator<T> - returns negative/zero/positive
typedef int (*LIBMATTI_JU_Comparator)(const void *a, const void *b);

// Java: public int compare(String s1, String s2) - String::compareTo
int LIBMATTI_JU_Comparator_StringCompare(const void *a, const void *b);
// Java: String::compareToIgnoreCase
int LIBMATTI_JU_Comparator_StringCompareIgnoreCase(const void *a, const void *b);
// Java: Comparator.reverseOrder() over strings
int LIBMATTI_JU_Comparator_StringReverse(const void *a, const void *b);
// Java: Comparator.comparingInt - the port's generic helper for size_t keys
int LIBMATTI_JU_Comparator_SizeTBy(const void *a, const void *b);

// Java: Comparator.reverseOrder() - wraps a comparator, heap-allocated struct:
typedef struct
{
    LIBMATTI_JU_Comparator wrapped;
} LIBMATTI_JU_ComparatorHolder;
LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_Reverse(LIBMATTI_JU_Comparator comparator);
int LIBMATTI_JU_Comparator_ReverseFn(const void *a, const void *b); // holder in a/b

// Java: Comparator.nullsFirst / nullsLast (NULL pointers sort first/last)
LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_NullsFirst(LIBMATTI_JU_Comparator comparator);
LIBMATTI_JU_ComparatorHolder *LIBMATTI_JU_Comparator_NullsLast(LIBMATTI_JU_Comparator comparator);
int LIBMATTI_JU_Comparator_NullsFirstFn(const void *a, const void *b);
int LIBMATTI_JU_Comparator_NullsLastFn(const void *a, const void *b);

// Java: Collections.sort(List<T>, Comparator<T>)
void LIBMATTI_JU_Collections_Sort(struct LIBMATTI_JU_ArrayList *list, LIBMATTI_JU_Comparator comparator);
// Java: Collections.reverse(List<T>)
void LIBMATTI_JU_Collections_Reverse(struct LIBMATTI_JU_ArrayList *list);
// Java: Collections.swap(List<T>, i, j) - exposed because the sorter uses it
void LIBMATTI_JU_Collections_Swap(struct LIBMATTI_JU_ArrayList *list, size_t i, size_t j);

#endif //MATTICRAFT_JAVA_UTIL_COMPARATOR_H
