// Port of cpw.mods.modlauncher.api.ITransformer.

#include "libmatti/cpw/modlauncher/api/ITransformer.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_MLA_ITransformer_Target *target_new(const char *className, const char *elementName,
                                                    const char *elementDescriptor,
                                                    const LIBMATTI_MLA_TargetType *targetType)
{
    LIBMATTI_MLA_ITransformer_Target *target = calloc(1, sizeof(LIBMATTI_MLA_ITransformer_Target));
    target->className = strdup(className);
    target->elementName = strdup(elementName);
    target->elementDescriptor = strdup(elementDescriptor);
    target->targetType = targetType;
    return target;
}

LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Class(const char *className)
{
    return target_new(className, "", "", LIBMATTI_MLA_TargetType_Class());
}

LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_PreClass(const char *className)
{
    return target_new(className, "", "", LIBMATTI_MLA_TargetType_PreClass());
}

LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Method(const char *className, const char *methodName,
                                                                         const char *methodDescriptor)
{
    return target_new(className, methodName, methodDescriptor, LIBMATTI_MLA_TargetType_Method());
}

LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Field(const char *className, const char *fieldName)
{
    return target_new(className, fieldName, "", LIBMATTI_MLA_TargetType_Field());
}

void LIBMATTI_MLA_ITransformer_Target_Free(LIBMATTI_MLA_ITransformer_Target *target)
{
    if (target == NULL) return;
    free(target->className);
    free(target->elementName);
    free(target->elementDescriptor);
    free(target);
}

char **LIBMATTI_MLA_ITransformer_DefaultLabel(size_t *count)
{
    static char *label[1];
    label[0] = "default";
    *count = 1;
    return label;
}

void *LIBMATTI_MLA_ITransformer_Transform(LIBMATTI_MLA_ITransformer *transformer, void *input,
                                          LIBMATTI_MLA_ITransformerVotingContext *context)
{
    return transformer->transform(transformer, input, context);
}

LIBMATTI_MLA_TransformerVoteResult LIBMATTI_MLA_ITransformer_CastVote(
    LIBMATTI_MLA_ITransformer *transformer, LIBMATTI_MLA_ITransformerVotingContext *context)
{
    return transformer->castVote(transformer, context);
}

LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Targets(LIBMATTI_MLA_ITransformer *transformer,
                                                                    size_t *count)
{
    return transformer->targets(transformer, count);
}

const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_ITransformer_GetTargetType(LIBMATTI_MLA_ITransformer *transformer)
{
    return transformer->getTargetType(transformer);
}

char **LIBMATTI_MLA_ITransformer_Labels(LIBMATTI_MLA_ITransformer *transformer, size_t *count)
{
    // Java: default String[] labels() { return DEFAULT_LABEL; }
    if (transformer->labels == NULL) return LIBMATTI_MLA_ITransformer_DefaultLabel(count);
    return transformer->labels(transformer, count);
}
