// Port of net.neoforged.fml.loading.modscan.ModMethodVisitor.

#include "libmatti/net/neoforged/fml/loading/modscan/ModMethodVisitor.h"

#include <stdlib.h>
#include <string.h>

// Java: public ModMethodVisitor(String name, String desc, LinkedList<ModAnnotation> annotations)
LIBMATTI_FML_ModMethodVisitor *LIBMATTI_FML_ModMethodVisitor_New(const char *name, const char *desc,
                                                                 LIBMATTI_FML_ModAnnotationList *annotations)
{
    LIBMATTI_FML_ModMethodVisitor *visitor = calloc(1, sizeof(LIBMATTI_FML_ModMethodVisitor));
    visitor->methodName = name != NULL ? strdup(name) : NULL;
    visitor->methodDescriptor = desc != NULL ? strdup(desc) : NULL;
    visitor->annotations = annotations;
    return visitor;
}

void LIBMATTI_FML_ModMethodVisitor_Free(LIBMATTI_FML_ModMethodVisitor *visitor)
{
    if (visitor == NULL) return;

    free(visitor->methodName);
    free(visitor->methodDescriptor);
    free(visitor);
}

// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModMethodVisitor_VisitAnnotation(
    LIBMATTI_FML_ModMethodVisitor *visitor, const char *annotationName, int runtimeVisible)
{
    (void) runtimeVisible;

    // Java: methodName + methodDescriptor
    size_t nameLength = visitor->methodName != NULL ? strlen(visitor->methodName) : 0;
    size_t descLength = visitor->methodDescriptor != NULL ? strlen(visitor->methodDescriptor) : 0;
    char *member = malloc(nameLength + descLength + 1);
    memcpy(member, visitor->methodName, nameLength);
    memcpy(member + nameLength, visitor->methodDescriptor, descLength);
    member[nameLength + descLength] = '\0';

    LIBMATTI_FML_ModAnnotation *annotation = LIBMATTI_FML_ModAnnotation_New(
        LIBMATTI_NEOFORGESPI_ElementType_METHOD, LIBMATTI_ASM_Type_GetType(annotationName), member);
    LIBMATTI_FML_ModAnnotationList_AddFirst(visitor->annotations, annotation);
    free(member);

    return LIBMATTI_FML_ModAnnotationVisitor_New(visitor->annotations, annotation);
}
