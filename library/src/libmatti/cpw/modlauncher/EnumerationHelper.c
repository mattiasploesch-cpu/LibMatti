// Port of cpw.mods.modlauncher.EnumerationHelper.

#include "libmatti/cpw/modlauncher/EnumerationHelper.h"

#include <stdlib.h>

// Java: static <T> Enumeration<T> merge(Enumeration<T> first, Enumeration<T> second)
LIBMATTI_ML_Enumeration LIBMATTI_ML_EnumerationHelper_Merge(LIBMATTI_ML_Enumeration first,
                                                            LIBMATTI_ML_Enumeration second)
{
    // Java: hasMoreElements() { return first.hasMoreElements() || second.hasMoreElements(); }
    //       nextElement() { return first.hasMoreElements() ? first.nextElement() : second.nextElement(); }
    LIBMATTI_ML_Enumeration merged;
    merged.count = (first.count - first.position) + (second.count - second.position);
    merged.position = 0;
    merged.items = malloc(sizeof(void *) * (merged.count > 0 ? merged.count : 1));

    size_t index = 0;
    for (size_t i = first.position; i < first.count; i++) merged.items[index++] = first.items[i];
    for (size_t i = second.position; i < second.count; i++) merged.items[index++] = second.items[i];

    return merged;
}

void *LIBMATTI_ML_EnumerationHelper_FirstElementOrNull(LIBMATTI_ML_Enumeration enumeration)
{
    if (enumeration.position >= enumeration.count) return NULL;
    return enumeration.items[enumeration.position];
}
