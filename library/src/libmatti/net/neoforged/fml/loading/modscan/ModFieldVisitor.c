// Port of net.neoforged.fml.loading.modscan.ModFieldVisitor.

#include "libmatti/net/neoforged/fml/loading/modscan/ModFieldVisitor.h"

#include <stdlib.h>
#include <string.h>

// Java: public ModFieldVisitor(String name, LinkedList<ModAnnotation> annotations)
LIBMATTI_FML_ModFieldVisitor *LIBMATTI_FML_ModFieldVisitor_New(const char *name,
                                                               LIBMATTI_FML_ModAnnotationList *annotations)
{
    LIBMATTI_FML_ModFieldVisitor *visitor = calloc(1, sizeof(LIBMATTI_FML_ModFieldVisitor));
    visitor->fieldName = name != NULL ? strdup(name) : NULL;
    visitor->annotations = annotations;
    return visitor;
}

void LIBMATTI_FML_ModFieldVisitor_Free(LIBMATTI_FML_ModFieldVisitor *visitor)
{
    if (visitor == NULL) return;

    free(visitor->fieldName);
    free(visitor);
}

// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModFieldVisitor_VisitAnnotation(
    LIBMATTI_FML_ModFieldVisitor *visitor, const char *annotationName, int runtimeVisible)
{
    (void) runtimeVisible;

    LIBMATTI_FML_ModAnnotation *annotation = LIBMATTI_FML_ModAnnotation_New(
        LIBMATTI_NEOFORGESPI_ElementType_FIELD, LIBMATTI_ASM_Type_GetType(annotationName),
        visitor->fieldName);
    LIBMATTI_FML_ModAnnotationList_AddFirst(visitor->annotations, annotation);

    return LIBMATTI_FML_ModAnnotationVisitor_New(visitor->annotations, annotation);
}
