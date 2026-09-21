// Port of cpw.mods.modlauncher.TransformStore.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMSTORE_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMSTORE_H

#include "libmatti/cpw/modlauncher/TransformList.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"
#include "libmatti/org/objectweb/asm/tree/FieldNode.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stddef.h>

// Java: public class TransformStore
typedef struct LIBMATTI_ML_TransformStore
{
    // Java: Set<String> classNeedsTransforming
    char **classNeedsTransforming;
    size_t classCount;
    // Java: Map<TargetType<?>, TransformList<?>> transformers (an EnumMap-shaped array)
    LIBMATTI_ML_TransformList *transformers[4];
} LIBMATTI_ML_TransformStore;

// Java: public TransformStore()
LIBMATTI_ML_TransformStore *LIBMATTI_ML_TransformStore_New(void);
void LIBMATTI_ML_TransformStore_Free(LIBMATTI_ML_TransformStore *store);

// Java: List<ITransformer<FieldNode>> getTransformersFor(String className, FieldNode field)
LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForField(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_ASMT_FieldNode *field, size_t *count);
// Java: List<ITransformer<MethodNode>> getTransformersFor(String className, MethodNode method)
LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForMethod(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_ASMT_MethodNode *method, size_t *count);
// Java: List<ITransformer<ClassNode>> getTransformersFor(String className, TargetType<ClassNode> classType)
LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForClass(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_MLA_TargetType *classType, size_t *count);

// Java: <T> void addTransformer(TransformTargetLabel, ITransformer<T>, ITransformationService)
void LIBMATTI_ML_TransformStore_AddTransformer(LIBMATTI_ML_TransformStore *store,
                                               LIBMATTI_ML_TransformTargetLabel *targetLabel,
                                               LIBMATTI_MLA_ITransformer *transformer,
                                               LIBMATTI_MLA_ITransformationService *service);

// Java: boolean needsTransforming(String internalClassName)
int LIBMATTI_ML_TransformStore_NeedsTransforming(const LIBMATTI_ML_TransformStore *store, const char *internalClassName);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMSTORE_H
