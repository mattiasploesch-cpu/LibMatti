// Port of net.neoforged.fml.classloading.transformation.TransformerClassWriter.
// Java's writer only asks getCommonSuperClass for the hierarchy when COMPUTE_FRAMES is set. The C
// ClassWriter does not recompute frames (see org/objectweb/asm/ClassWriter.h), so the hierarchy
// helpers below are provided for callers that need them, but the writer itself never calls them.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMERCLASSWRITER_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMERCLASSWRITER_H

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassHierarchyRecomputationContext.h"
#include "libmatti/org/objectweb/asm/ClassWriter.h"

// Java: class TransformerClassWriter extends ClassWriter
typedef struct LIBMATTI_FML_TransformerClassWriter
{
    // Java: extends ClassWriter
    LIBMATTI_ASM_ClassWriter *writer;

    LIBMATTI_ASMT_ClassNode *clazzAccessor;
    // Java: private boolean computedThis = false
    int computedThis;
    LIBMATTI_FML_ClassHierarchyRecomputationContext *recomputationContext;
} LIBMATTI_FML_TransformerClassWriter;

// Java: public TransformerClassWriter(int writerFlags, ClassNode clazzAccessor, ClassHierarchyRecomputationContext recomputationContext)
LIBMATTI_FML_TransformerClassWriter *LIBMATTI_FML_TransformerClassWriter_New(
    int writerFlags, LIBMATTI_ASMT_ClassNode *clazzAccessor,
    LIBMATTI_FML_ClassHierarchyRecomputationContext *recomputationContext);
void LIBMATTI_FML_TransformerClassWriter_Free(LIBMATTI_FML_TransformerClassWriter *writer);

// Java: protected String getCommonSuperClass(String type1, String type2); caller frees
char *LIBMATTI_FML_TransformerClassWriter_GetCommonSuperClass(LIBMATTI_FML_TransformerClassWriter *writer,
                                                             const char *type1, const char *type2);
// Java: private void computeHierarchy(ClassNode clazzNode)
void LIBMATTI_FML_TransformerClassWriter_ComputeHierarchyForClassNode(LIBMATTI_FML_TransformerClassWriter *writer,
                                                                     LIBMATTI_ASMT_ClassNode *clazzNode);
// Java: private void computeHierarchy(String className)
void LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(LIBMATTI_FML_TransformerClassWriter *writer,
                                                         const char *className);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMERCLASSWRITER_H
