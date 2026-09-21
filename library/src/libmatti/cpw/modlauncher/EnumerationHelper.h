// Port of cpw.mods.modlauncher.EnumerationHelper.

#ifndef MATTICRAFT_MODLAUNCHER_ENUMERATIONHELPER_H
#define MATTICRAFT_MODLAUNCHER_ENUMERATIONHELPER_H

#include <stddef.h>

// Java: java.util.Enumeration<T> - expressed as a cursor over an array
typedef struct
{
    void **items;
    size_t count;
    size_t position;
} LIBMATTI_ML_Enumeration;

// Java: static <T> Enumeration<T> merge(Enumeration<T> first, Enumeration<T> second)
LIBMATTI_ML_Enumeration LIBMATTI_ML_EnumerationHelper_Merge(LIBMATTI_ML_Enumeration first,
                                                            LIBMATTI_ML_Enumeration second);
// Java: static <T> T firstElementOrNull(Enumeration<T> enumeration)
void *LIBMATTI_ML_EnumerationHelper_FirstElementOrNull(LIBMATTI_ML_Enumeration enumeration);

#endif //MATTICRAFT_MODLAUNCHER_ENUMERATIONHELPER_H
