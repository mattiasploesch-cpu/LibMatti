// Port of net.neoforged.fml.loading.modscan.ModMethodVisitor.

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_MODMETHODVISITOR_H
#define MATTICRAFT_FML_LOADING_MODSCAN_MODMETHODVISITOR_H

#include "libmatti/net/neoforged/fml/loading/modscan/ModAnnotationVisitor.h"

typedef struct LIBMATTI_FML_ModMethodVisitor LIBMATTI_FML_ModMethodVisitor;

struct LIBMATTI_FML_ModMethodVisitor
{
    LIBMATTI_FML_ModAnnotationList *annotations;
    char *methodName;
    char *methodDescriptor;
};

// Java: public ModMethodVisitor(String name, String desc, LinkedList<ModAnnotation> annotations)
LIBMATTI_FML_ModMethodVisitor *LIBMATTI_FML_ModMethodVisitor_New(const char *name, const char *desc,
                                                                 LIBMATTI_FML_ModAnnotationList *annotations);
void LIBMATTI_FML_ModMethodVisitor_Free(LIBMATTI_FML_ModMethodVisitor *visitor);

// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModMethodVisitor_VisitAnnotation(
    LIBMATTI_FML_ModMethodVisitor *visitor, const char *annotationName, int runtimeVisible);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_MODMETHODVISITOR_H
