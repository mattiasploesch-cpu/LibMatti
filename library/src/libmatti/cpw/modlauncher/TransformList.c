// Port of cpw.mods.modlauncher.TransformList.
// Java's ConcurrentHashMap keeps the key object that was first inserted; the C
// port therefore takes ownership of the TransformTargetLabel passed in.

#include "libmatti/cpw/modlauncher/TransformList.h"

#include <stdlib.h>

LIBMATTI_ML_TransformList *LIBMATTI_ML_TransformList_New(const char *nodeType)
{
    LIBMATTI_ML_TransformList *list = calloc(1, sizeof(LIBMATTI_ML_TransformList));
    list->nodeType = nodeType;
    return list;
}

void LIBMATTI_ML_TransformList_Free(LIBMATTI_ML_TransformList *list)
{
    if (list == NULL) return;

    for (size_t i = 0; i < list->count; i++)
    {
        LIBMATTI_ML_TransformTargetLabel_Free(list->entries[i].label);
        free(list->entries[i].transformers);
    }
    free(list->entries);
    free(list);
}

// Java: transformers.computeIfAbsent(targetLabel, v -> new ArrayList<>())
static LIBMATTI_ML_TransformListEntry *entry_for(LIBMATTI_ML_TransformList *list,
                                                 LIBMATTI_ML_TransformTargetLabel *label)
{
    for (size_t i = 0; i < list->count; i++)
    {
        if (!LIBMATTI_ML_TransformTargetLabel_Equals(list->entries[i].label, label)) continue;
        // an equal key is already present, so the map does not take the new one
        LIBMATTI_ML_TransformTargetLabel_Free(label);
        return &list->entries[i];
    }

    list->entries = realloc(list->entries, sizeof(*list->entries) * (list->count + 1));
    LIBMATTI_ML_TransformListEntry *entry = &list->entries[list->count++];
    entry->label = label;
    entry->transformers = NULL;
    entry->count = 0;
    return entry;
}

void LIBMATTI_ML_TransformList_AddTransformer(LIBMATTI_ML_TransformList *list,
                                              LIBMATTI_ML_TransformTargetLabel *targetLabel,
                                              LIBMATTI_MLA_ITransformer *transformer)
{
    LIBMATTI_ML_TransformListEntry *entry = entry_for(list, targetLabel);
    entry->transformers = realloc(entry->transformers, sizeof(*entry->transformers) * (entry->count + 1));
    entry->transformers[entry->count++] = transformer;
}

LIBMATTI_MLA_ITransformer **LIBMATTI_ML_TransformList_GetTransformersForLabel(
    LIBMATTI_ML_TransformList *list, LIBMATTI_ML_TransformTargetLabel *label, size_t *count)
{
    LIBMATTI_ML_TransformListEntry *entry = entry_for(list, label);
    *count = entry->count;
    return entry->transformers;
}
