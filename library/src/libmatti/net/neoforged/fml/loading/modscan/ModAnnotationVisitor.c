// Port of net.neoforged.fml.loading.modscan.ModAnnotationVisitor.

#include "libmatti/net/neoforged/fml/loading/modscan/ModAnnotationVisitor.h"

#include <stdlib.h>

// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_New(LIBMATTI_FML_ModAnnotationList *annotations,
                                                                        LIBMATTI_FML_ModAnnotation *annotation)
{
    LIBMATTI_FML_ModAnnotationVisitor *visitor = calloc(1, sizeof(LIBMATTI_FML_ModAnnotationVisitor));
    visitor->annotations = annotations;
    visitor->annotation = annotation;
    return visitor;
}

// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation, String name)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_NewArray(
    LIBMATTI_FML_ModAnnotationList *annotations, LIBMATTI_FML_ModAnnotation *annotation, const char *name)
{
    LIBMATTI_FML_ModAnnotationVisitor *visitor =
        LIBMATTI_FML_ModAnnotationVisitor_New(annotations, annotation);
    visitor->array = 1;
    LIBMATTI_FML_ModAnnotation_AddArray(annotation, name);
    return visitor;
}

// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation, boolean isSubAnnotation)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_NewSubAnnotation(
    LIBMATTI_FML_ModAnnotationList *annotations, LIBMATTI_FML_ModAnnotation *annotation)
{
    LIBMATTI_FML_ModAnnotationVisitor *visitor =
        LIBMATTI_FML_ModAnnotationVisitor_New(annotations, annotation);
    visitor->isSubAnnotation = 1;
    return visitor;
}

void LIBMATTI_FML_ModAnnotationVisitor_Free(LIBMATTI_FML_ModAnnotationVisitor *visitor)
{
    free(visitor);
}

// Java: public void visit(String key, Object value)
void LIBMATTI_FML_ModAnnotationVisitor_Visit(LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *key,
                                             LIBMATTI_FML_ModAnnotationValue *value)
{
    LIBMATTI_FML_ModAnnotation_AddProperty(visitor->annotation, key, value);
}

// Java: public void visitEnum(String name, String desc, String value)
void LIBMATTI_FML_ModAnnotationVisitor_VisitEnum(LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name,
                                                 const char *desc, const char *value)
{
    LIBMATTI_FML_ModAnnotation_AddEnumProperty(visitor->annotation, name, desc, value);
}

// Java: public AnnotationVisitor visitArray(String name)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_VisitArray(
    LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name)
{
    return LIBMATTI_FML_ModAnnotationVisitor_NewArray(visitor->annotations, visitor->annotation, name);
}

// Java: public AnnotationVisitor visitAnnotation(String name, String desc)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_VisitAnnotation(
    LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name, const char *desc)
{
    LIBMATTI_FML_ModAnnotation *current = LIBMATTI_FML_ModAnnotationList_GetFirst(visitor->annotations);
    LIBMATTI_FML_ModAnnotation *child = LIBMATTI_FML_ModAnnotation_AddChildAnnotation(current, name, desc);
    LIBMATTI_FML_ModAnnotationList_AddFirst(visitor->annotations, child);

    return LIBMATTI_FML_ModAnnotationVisitor_NewSubAnnotation(visitor->annotations, child);
}

// Java: public void visitEnd()
void LIBMATTI_FML_ModAnnotationVisitor_VisitEnd(LIBMATTI_FML_ModAnnotationVisitor *visitor)
{
    if (visitor->array) LIBMATTI_FML_ModAnnotation_EndArray(visitor->annotation);

    if (visitor->isSubAnnotation)
    {
        LIBMATTI_FML_ModAnnotation *child = LIBMATTI_FML_ModAnnotationList_RemoveFirst(visitor->annotations);
        LIBMATTI_FML_ModAnnotationList_AddLast(visitor->annotations, child);
    }
}
