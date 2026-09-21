// Port of cpw.mods.modlauncher.TransformList.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMLIST_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMLIST_H

#include "libmatti/cpw/modlauncher/TransformTargetLabel.h"

#include <stddef.h>

// Java: Map<TransformTargetLabel, List<ITransformer<T>>> transformers
typedef struct
{
    LIBMATTI_ML_TransformTargetLabel *label;
    LIBMATTI_MLA_ITransformer **transformers;
    size_t count;
} LIBMATTI_ML_TransformListEntry;

// Java: public class TransformList<T>
typedef struct LIBMATTI_ML_TransformList
{
    LIBMATTI_ML_TransformListEntry *entries;
    size_t count;
    // Java: Class<T> nodeType
    const char *nodeType;
} LIBMATTI_ML_TransformList;

// Java: TransformList(Class<T> nodeType)
LIBMATTI_ML_TransformList *LIBMATTI_ML_TransformList_New(const char *nodeType);
void LIBMATTI_ML_TransformList_Free(LIBMATTI_ML_TransformList *list);

// Java: void addTransformer(TransformTargetLabel, ITransformer<T>)
// Ownership of targetLabel moves into the list.
void LIBMATTI_ML_TransformList_AddTransformer(LIBMATTI_ML_TransformList *list,
                                              LIBMATTI_ML_TransformTargetLabel *targetLabel,
                                              LIBMATTI_MLA_ITransformer *transformer);
// Java: List<ITransformer<T>> getTransformersForLabel(TransformTargetLabel label)
// Ownership of label moves into the list when the label is new.
LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformList_GetTransformersForLabel(
    LIBMATTI_ML_TransformList *list, LIBMATTI_ML_TransformTargetLabel *label, size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMLIST_H
