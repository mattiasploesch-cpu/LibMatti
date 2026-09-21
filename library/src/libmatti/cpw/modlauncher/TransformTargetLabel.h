// Port of cpw.mods.modlauncher.TransformTargetLabel.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMTARGETLABEL_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMTARGETLABEL_H

#include "libmatti/cpw/modlauncher/api/ITransformer.h"

// Java: public final class TransformTargetLabel
typedef struct LIBMATTI_ML_TransformTargetLabel
{
    LIBMATTI_ASM_Type *className;
    char *elementName;
    LIBMATTI_ASM_Type *elementDescriptor;
    const LIBMATTI_MLA_TargetType *labelType;
} LIBMATTI_ML_TransformTargetLabel;

// Java: TransformTargetLabel(ITransformer.Target<?> target)
LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewFromTarget(
    const LIBMATTI_MLA_ITransformer_Target *target);
// Java: public TransformTargetLabel(String className, String fieldName) - FIELD
LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewField(const char *className, const char *fieldName);
// Java: TransformTargetLabel(String className, String methodName, String methodDesc) - METHOD
LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewMethod(const char *className, const char *methodName,
                                                                            const char *methodDesc);
// Java: public TransformTargetLabel(String className) - CLASS
LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewClass(const char *className);
// Java: public TransformTargetLabel(String className, TargetType<ClassNode> type)
LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewClassWithType(
    const char *className, const LIBMATTI_MLA_TargetType *type);

// Java: Type getClassName()
LIBMATTI_ASM_Type *LIBMATTI_ML_TransformTargetLabel_GetClassName(const LIBMATTI_ML_TransformTargetLabel *label);
// Java: public final String getElementName()
const char *LIBMATTI_ML_TransformTargetLabel_GetElementName(const LIBMATTI_ML_TransformTargetLabel *label);
// Java: public final Type getElementDescriptor()
LIBMATTI_ASM_Type *LIBMATTI_ML_TransformTargetLabel_GetElementDescriptor(const LIBMATTI_ML_TransformTargetLabel *label);
// Java: TargetType<?> getTargetType()
const LIBMATTI_MLA_TargetType *LIBMATTI_ML_TransformTargetLabel_GetTargetType(
    const LIBMATTI_ML_TransformTargetLabel *label);

// Java: public int hashCode() / public boolean equals(Object)
int LIBMATTI_ML_TransformTargetLabel_HashCode(const LIBMATTI_ML_TransformTargetLabel *label);
int LIBMATTI_ML_TransformTargetLabel_Equals(const LIBMATTI_ML_TransformTargetLabel *label,
                                            const LIBMATTI_ML_TransformTargetLabel *other);
// Java: public String toString(); caller frees
char *LIBMATTI_ML_TransformTargetLabel_ToString(const LIBMATTI_ML_TransformTargetLabel *label);

void LIBMATTI_ML_TransformTargetLabel_Free(LIBMATTI_ML_TransformTargetLabel *label);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMTARGETLABEL_H
