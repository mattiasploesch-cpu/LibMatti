// Port of net.neoforged.fml.loading.modscan.ModAnnotationVisitor.
// Java's three constructors become the three New functions below; the array element
// name a ModAnnotationVisitor keeps in a field it never reads is not carried over.

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATIONVISITOR_H
#define MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATIONVISITOR_H

#include "libmatti/net/neoforged/fml/loading/modscan/ModAnnotation.h"

typedef struct LIBMATTI_FML_ModAnnotationVisitor LIBMATTI_FML_ModAnnotationVisitor;

struct LIBMATTI_FML_ModAnnotationVisitor
{
    LIBMATTI_FML_ModAnnotationList *annotations;
    LIBMATTI_FML_ModAnnotation *annotation;
    int array;
    int isSubAnnotation;
};

// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_New(LIBMATTI_FML_ModAnnotationList *annotations,
                                                                        LIBMATTI_FML_ModAnnotation *annotation);
// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation, String name)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_NewArray(
    LIBMATTI_FML_ModAnnotationList *annotations, LIBMATTI_FML_ModAnnotation *annotation, const char *name);
// Java: public ModAnnotationVisitor(LinkedList<ModAnnotation> annotations, ModAnnotation annotation, boolean isSubAnnotation)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_NewSubAnnotation(
    LIBMATTI_FML_ModAnnotationList *annotations, LIBMATTI_FML_ModAnnotation *annotation);

// Java: the visitor is garbage collected; the port frees it after visitEnd()
void LIBMATTI_FML_ModAnnotationVisitor_Free(LIBMATTI_FML_ModAnnotationVisitor *visitor);

// Java: public void visit(String key, Object value)
void LIBMATTI_FML_ModAnnotationVisitor_Visit(LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *key,
                                             LIBMATTI_FML_ModAnnotationValue *value);
// Java: public void visitEnum(String name, String desc, String value)
void LIBMATTI_FML_ModAnnotationVisitor_VisitEnum(LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name,
                                                 const char *desc, const char *value);
// Java: public AnnotationVisitor visitArray(String name)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_VisitArray(
    LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name);
// Java: public AnnotationVisitor visitAnnotation(String name, String desc)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModAnnotationVisitor_VisitAnnotation(
    LIBMATTI_FML_ModAnnotationVisitor *visitor, const char *name, const char *desc);
// Java: public void visitEnd()
void LIBMATTI_FML_ModAnnotationVisitor_VisitEnd(LIBMATTI_FML_ModAnnotationVisitor *visitor);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATIONVISITOR_H
