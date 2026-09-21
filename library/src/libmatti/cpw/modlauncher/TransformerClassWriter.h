// Port of cpw.mods.modlauncher.TransformerClassWriter.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMERCLASSWRITER_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMERCLASSWRITER_H

#include "libmatti/cpw/modlauncher/ClassTransformer.h"
#include "libmatti/org/objectweb/asm/ClassWriter.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

// Java: public static ClassWriter createClassWriter(int mlFlags, ClassTransformer classTransformer, ClassNode clazzAccessor)
LIBMATTI_ASM_ClassWriter *LIBMATTI_ML_TransformerClassWriter_CreateClassWriter(
    int mlFlags, LIBMATTI_ML_ClassTransformer *classTransformer, LIBMATTI_ASMT_ClassNode *clazzAccessor);

// Java: protected String getCommonSuperClass(String type1, String type2); caller frees
char *LIBMATTI_ML_TransformerClassWriter_GetCommonSuperClass(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                             const char *type1, const char *type2);

// Java: void computeHierarchy(ClassNode clazzNode)
void LIBMATTI_ML_TransformerClassWriter_ComputeHierarchyForClassNode(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                                     LIBMATTI_ASMT_ClassNode *clazzNode);
// Java: void computeHierarchy(String className)
void LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                         const char *className);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMERCLASSWRITER_H
