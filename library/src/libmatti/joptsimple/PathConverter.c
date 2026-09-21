// Port of joptsimple.util.PathConverter.

#include "libmatti/joptsimple/PathConverter.h"

#include <stdlib.h>

LIBMATTI_JOPT_PathConverter *LIBMATTI_JOPT_PathConverter_New(LIBMATTI_JOPT_PathProperties property)
{
    LIBMATTI_JOPT_PathConverter *converter = calloc(1, sizeof(LIBMATTI_JOPT_PathConverter));
    converter->property = property;
    return converter;
}

void LIBMATTI_JOPT_PathConverter_Free(LIBMATTI_JOPT_PathConverter *converter)
{
    free(converter);
}
