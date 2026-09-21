// Port of net.neoforged.fml.loading.modscan.ModFieldVisitor.
// Java extends FieldVisitor; the port's ClassReader only accepts a ClassNode, so the
// visitor is driven from the node's annotation list (see ModClassVisitor_VisitNode).

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_MODFIELDVISITOR_H
#define MATTICRAFT_FML_LOADING_MODSCAN_MODFIELDVISITOR_H

#include "libmatti/net/neoforged/fml/loading/modscan/ModAnnotationVisitor.h"

typedef struct LIBMATTI_FML_ModFieldVisitor LIBMATTI_FML_ModFieldVisitor;

struct LIBMATTI_FML_ModFieldVisitor
{
    LIBMATTI_FML_ModAnnotationList *annotations;
    char *fieldName;
};

// Java: public ModFieldVisitor(String name, LinkedList<ModAnnotation> annotations)
LIBMATTI_FML_ModFieldVisitor *LIBMATTI_FML_ModFieldVisitor_New(const char *name,
                                                               LIBMATTI_FML_ModAnnotationList *annotations);
void LIBMATTI_FML_ModFieldVisitor_Free(LIBMATTI_FML_ModFieldVisitor *visitor);

// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModFieldVisitor_VisitAnnotation(
    LIBMATTI_FML_ModFieldVisitor *visitor, const char *annotationName, int runtimeVisible);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_MODFIELDVISITOR_H
