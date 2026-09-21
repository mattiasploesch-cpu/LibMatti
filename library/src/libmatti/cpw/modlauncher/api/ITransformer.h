// Port of cpw.mods.modlauncher.api.ITransformer.
// Java uses generics (T is ClassNode/MethodNode/FieldNode); the C port passes the
// node as a void* and the target type carries which node class it is.

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMER_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMER_H

#include "libmatti/cpw/modlauncher/api/ITransformerVotingContext.h"
#include "libmatti/cpw/modlauncher/api/TargetType.h"
#include "libmatti/cpw/modlauncher/api/TransformerVoteResult.h"

#include <stddef.h>

// Java: record Target<T>(String className, String elementName, String elementDescriptor, TargetType<T> targetType)
typedef struct
{
    char *className;
    char *elementName;
    char *elementDescriptor;
    const LIBMATTI_MLA_TargetType *targetType;
} LIBMATTI_MLA_ITransformer_Target;

// Java: the static Target factories
LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Class(const char *className);
LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_PreClass(const char *className);
LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Method(const char *className, const char *methodName,
                                                                          const char *methodDescriptor);
LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Target_Field(const char *className, const char *fieldName);
void LIBMATTI_MLA_ITransformer_Target_Free(LIBMATTI_MLA_ITransformer_Target *target);

// Java: interface ITransformer<T>
typedef struct LIBMATTI_MLA_ITransformer
{
    // Java: T transform(T input, ITransformerVotingContext context)
    void *(*transform)(struct LIBMATTI_MLA_ITransformer *self, void *input,
                       LIBMATTI_MLA_ITransformerVotingContext *context);
    // Java: TransformerVoteResult castVote(ITransformerVotingContext context)
    LIBMATTI_MLA_TransformerVoteResult (*castVote)(struct LIBMATTI_MLA_ITransformer *self,
                                                   LIBMATTI_MLA_ITransformerVotingContext *context);
    // Java: Set<Target<T>> targets()
    LIBMATTI_MLA_ITransformer_Target *(*targets)(struct LIBMATTI_MLA_ITransformer *self, size_t *count);
    // Java: TargetType<T> getTargetType()
    const LIBMATTI_MLA_TargetType *(*getTargetType)(struct LIBMATTI_MLA_ITransformer *self);
    // Java: default String[] labels() { return DEFAULT_LABEL; }
    char **(*labels)(struct LIBMATTI_MLA_ITransformer *self, size_t *count);
} LIBMATTI_MLA_ITransformer;

// Java: static final String[] DEFAULT_LABEL = {"default"}
char **LIBMATTI_MLA_ITransformer_DefaultLabel(size_t *count);

// Java: the interface methods, dispatched to the implementing vtable
void *LIBMATTI_MLA_ITransformer_Transform(LIBMATTI_MLA_ITransformer *transformer, void *input,
                                          LIBMATTI_MLA_ITransformerVotingContext *context);
LIBMATTI_MLA_TransformerVoteResult LIBMATTI_MLA_ITransformer_CastVote(
    LIBMATTI_MLA_ITransformer *transformer, LIBMATTI_MLA_ITransformerVotingContext *context);
LIBMATTI_MLA_ITransformer_Target *LIBMATTI_MLA_ITransformer_Targets(LIBMATTI_MLA_ITransformer *transformer,
                                                                     size_t *count);
const LIBMATTI_MLA_TargetType *LIBMATTI_MLA_ITransformer_GetTargetType(LIBMATTI_MLA_ITransformer *transformer);
char **LIBMATTI_MLA_ITransformer_Labels(LIBMATTI_MLA_ITransformer *transformer, size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMER_H
