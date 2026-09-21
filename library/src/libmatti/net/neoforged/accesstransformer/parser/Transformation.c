#include "libmatti/net/neoforged/accesstransformer/parser/Transformation.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_AT_Transformation *LIBMATTI_AT_Transformation_New(LIBMATTI_AT_Modifier modifier,
                                                           LIBMATTI_AT_FinalState finalState,
                                                           const char *origin, int line)
{
    LIBMATTI_AT_Transformation *transformation = calloc(1, sizeof(LIBMATTI_AT_Transformation));
    transformation->modifier = modifier;
    transformation->finalState = finalState;
    transformation->origin = origin != NULL ? strdup(origin) : NULL;
    transformation->line = line;
    return transformation;
}

void LIBMATTI_AT_Transformation_Free(LIBMATTI_AT_Transformation *transformation)
{
    if (transformation == NULL)
        return;
    free(transformation->origin);
    free(transformation);
}
