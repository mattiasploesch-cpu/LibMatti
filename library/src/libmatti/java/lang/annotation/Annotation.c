// Port of the java.lang.annotation model (see Annotation.h).

#include "libmatti/java/lang/annotation/Annotation.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_Annotation *LIBMATTI_JL_Annotation_New(const char *className, LIBMATTI_JL_RetentionPolicy retention)
{
    LIBMATTI_JL_Annotation *annotation = calloc(1, sizeof(LIBMATTI_JL_Annotation));
    annotation->className = strdup(className);
    annotation->retention = retention;
    return annotation;
}

void LIBMATTI_JL_Annotation_Free(LIBMATTI_JL_Annotation *annotation)
{
    if (annotation == NULL) return;
    free(annotation->className);
    free(annotation);
}
