// Port of net.neoforged.fml.loading.modscan.ModClassVisitor.
// Java extends ClassVisitor and is fed by ClassReader.accept; the port's ClassReader
// only accepts a ClassNode, so VisitNode below replays the ClassNode into the visitor.

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_MODCLASSVISITOR_H
#define MATTICRAFT_FML_LOADING_MODSCAN_MODCLASSVISITOR_H

#include "libmatti/net/neoforged/fml/loading/modscan/ModFieldVisitor.h"
#include "libmatti/net/neoforged/fml/loading/modscan/ModMethodVisitor.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

typedef struct LIBMATTI_FML_ModClassVisitor LIBMATTI_FML_ModClassVisitor;

struct LIBMATTI_FML_ModClassVisitor
{
    LIBMATTI_ASM_Type *asmType;
    LIBMATTI_ASM_Type *asmSuperType;
    LIBMATTI_ASM_Type **interfaces;
    size_t interfaceCount;
    LIBMATTI_FML_ModAnnotationList annotations;
};

// Java: public ModClassVisitor()
LIBMATTI_FML_ModClassVisitor *LIBMATTI_FML_ModClassVisitor_New(void);
void LIBMATTI_FML_ModClassVisitor_Free(LIBMATTI_FML_ModClassVisitor *visitor);

// Java: public void visit(int version, int access, String name, String signature, String superName, String[] interfaces)
void LIBMATTI_FML_ModClassVisitor_Visit(LIBMATTI_FML_ModClassVisitor *visitor, int version, int access,
                                        const char *name, const char *signature, const char *superName,
                                        char **interfaces, size_t interfaceCount);
// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModClassVisitor_VisitAnnotation(
    LIBMATTI_FML_ModClassVisitor *visitor, const char *annotationName, int runtimeVisible);
// Java: public FieldVisitor visitField(int access, String name, String desc, String signature, Object value)
LIBMATTI_FML_ModFieldVisitor *LIBMATTI_FML_ModClassVisitor_VisitField(
    LIBMATTI_FML_ModClassVisitor *visitor, int access, const char *name, const char *desc, const char *signature);
// Java: public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions)
LIBMATTI_FML_ModMethodVisitor *LIBMATTI_FML_ModClassVisitor_VisitMethod(
    LIBMATTI_FML_ModClassVisitor *visitor, int access, const char *name, const char *desc, const char *signature);
// Java: public void buildData(Set<ClassData> classes, Set<AnnotationData> annotations)
void LIBMATTI_FML_ModClassVisitor_BuildData(LIBMATTI_FML_ModClassVisitor *visitor,
                                            LIBMATTI_NEOFORGESPI_ModFileScanData *result);

// Java: ClassNode.accept(ClassVisitor) / FieldNode.accept / MethodNode.accept /
// AnnotationNode.accept - the classes are walked here because the port's ClassReader
// has no ClassVisitor interface to drive them.
void LIBMATTI_FML_ModClassVisitor_VisitNode(LIBMATTI_FML_ModClassVisitor *visitor,
                                            const LIBMATTI_ASMT_ClassNode *node);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_MODCLASSVISITOR_H
