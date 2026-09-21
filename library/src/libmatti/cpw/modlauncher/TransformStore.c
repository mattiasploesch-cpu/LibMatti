// Port of cpw.mods.modlauncher.TransformStore.

#include "libmatti/cpw/modlauncher/TransformStore.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "TransformerHolder.h"

#include <stdlib.h>
#include <string.h>

// Java: Map<TargetType<?>, TransformList<?>> - TargetType.VALUES order
static size_t target_index(const LIBMATTI_MLA_TargetType *type)
{
    size_t count = 0;
    const LIBMATTI_MLA_TargetType **values = LIBMATTI_MLA_TargetType_Values(&count);
    for (size_t i = 0; i < count; i++) if (values[i] == type || strcmp(values[i]->name, type->name) == 0) return i;
    return 0;
}

LIBMATTI_ML_TransformStore *LIBMATTI_ML_TransformStore_New(void)
{
    LIBMATTI_ML_TransformStore *store = calloc(1, sizeof(LIBMATTI_ML_TransformStore));

    // Java: for (TargetType<?> type : TargetType.VALUES) transformers.put(type, new TransformList<>(type.getNodeType()))
    size_t count = 0;
    const LIBMATTI_MLA_TargetType **values = LIBMATTI_MLA_TargetType_Values(&count);
    for (size_t i = 0; i < count; i++)
        store->transformers[i] = LIBMATTI_ML_TransformList_New(LIBMATTI_MLA_TargetType_GetNodeType(values[i]));

    return store;
}

void LIBMATTI_ML_TransformStore_Free(LIBMATTI_ML_TransformStore *store)
{
    if (store == NULL) return;

    for (size_t i = 0; i < 4; i++) LIBMATTI_ML_TransformList_Free(store->transformers[i]);
    for (size_t i = 0; i < store->classCount; i++) free(store->classNeedsTransforming[i]);
    free(store->classNeedsTransforming);
    free(store);
}

LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForField(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_ASMT_FieldNode *field, size_t *count)
{
    LIBMATTI_ML_TransformTargetLabel *label = LIBMATTI_ML_TransformTargetLabel_NewField(className, field->name);
    return LIBMATTI_ML_TransformList_GetTransformersForLabel(store->transformers[target_index(
                                                                  LIBMATTI_MLA_TargetType_Field())],
                                                              label, count);
}

LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForMethod(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_ASMT_MethodNode *method, size_t *count)
{
    LIBMATTI_ML_TransformTargetLabel *label = LIBMATTI_ML_TransformTargetLabel_NewMethod(className, method->name,
                                                                                          method->desc);
    return LIBMATTI_ML_TransformList_GetTransformersForLabel(store->transformers[target_index(
                                                                  LIBMATTI_MLA_TargetType_Method())],
                                                              label, count);
}

LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformStore_GetTransformersForClass(
    LIBMATTI_ML_TransformStore *store, const char *className, const LIBMATTI_MLA_TargetType *classType, size_t *count)
{
    LIBMATTI_ML_TransformTargetLabel *label = LIBMATTI_ML_TransformTargetLabel_NewClassWithType(className, classType);
    return LIBMATTI_ML_TransformList_GetTransformersForLabel(store->transformers[target_index(classType)], label, count);
}

void LIBMATTI_ML_TransformStore_AddTransformer(LIBMATTI_ML_TransformStore *store,
                                               LIBMATTI_ML_TransformTargetLabel *targetLabel,
                                               LIBMATTI_MLA_ITransformer *transformer,
                                               LIBMATTI_MLA_ITransformationService *service)
{
    char *labelString = LIBMATTI_ML_TransformTargetLabel_ToString(targetLabel);
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                             "Adding transformer {} to {}", "", labelString);
    free(labelString);

    // Java: classNeedsTransforming.add(targetLabel.getClassName().getInternalName())
    char *internalName = LIBMATTI_ASM_Type_GetInternalName(
        LIBMATTI_ML_TransformTargetLabel_GetClassName(targetLabel));

    int known = 0;
    for (size_t i = 0; i < store->classCount; i++)
        if (strcmp(store->classNeedsTransforming[i], internalName) == 0)
        {
            known = 1;
            break;
        }

    if (!known)
    {
        store->classNeedsTransforming = realloc(store->classNeedsTransforming,
                                                sizeof(*store->classNeedsTransforming) * (store->classCount + 1));
        store->classNeedsTransforming[store->classCount++] = internalName;
    }
    else
    {
        free(internalName);
    }

    // Java: new TransformerHolder<>(transformer, service)
    LIBMATTI_ML_TransformerHolder *holder = LIBMATTI_ML_TransformerHolder_New(transformer, service);
    LIBMATTI_ML_TransformList_AddTransformer(store->transformers[target_index(
                                                   LIBMATTI_ML_TransformTargetLabel_GetTargetType(targetLabel))],
                                             targetLabel, &holder->base);
}

int LIBMATTI_ML_TransformStore_NeedsTransforming(const LIBMATTI_ML_TransformStore *store, const char *internalClassName)
{
    for (size_t i = 0; i < store->classCount; i++)
        if (strcmp(store->classNeedsTransforming[i], internalClassName) == 0) return 1;
    return 0;
}
