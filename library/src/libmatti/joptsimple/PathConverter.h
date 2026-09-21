// Port of joptsimple.util.PathConverter.

#ifndef MATTICRAFT_JOPTSIMPLE_PATHCONVERTER_H
#define MATTICRAFT_JOPTSIMPLE_PATHCONVERTER_H

#include "libmatti/joptsimple/PathProperties.h"

// Java: public class PathConverter extends PathProperties implements ArgumentConverter
typedef struct LIBMATTI_JOPT_PathConverter
{
    LIBMATTI_JOPT_PathProperties property;
} LIBMATTI_JOPT_PathConverter;

// Java: public PathConverter(PathProperties... requiredProperties)
LIBMATTI_JOPT_PathConverter *LIBMATTI_JOPT_PathConverter_New(LIBMATTI_JOPT_PathProperties property);
void LIBMATTI_JOPT_PathConverter_Free(LIBMATTI_JOPT_PathConverter *converter);

#endif //MATTICRAFT_JOPTSIMPLE_PATHCONVERTER_H
