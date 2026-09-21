// Port of cpw.mods.modlauncher.TransformerHolder.

#include "libmatti/cpw/modlauncher/TransformerHolder.h"

#include <stdlib.h>

static void *holder_transform(LIBMATTI_MLA_ITransformer *self, void *input,
                              LIBMATTI_MLA_ITransformerVotingContext *context)
{
    LIBMATTI_ML_TransformerHolder *holder = (LIBMATTI_ML_TransformerHolder *)self;
    return LIBMATTI_MLA_ITransformer_Transform(holder->wrapped, input, context);
}

static LIBMATTI_MLA_TransformerVoteResult holder_cast_vote(LIBMATTI_MLA_ITransformer *self,
                                                           LIBMATTI_MLA_ITransformerVotingContext *context)
{
    LIBMATTI_ML_TransformerHolder *holder = (LIBMATTI_ML_TransformerHolder *)self;
    return LIBMATTI_MLA_ITransformer_CastVote(holder->wrapped, context);
}

static LIBMATTI_MLA_ITransformer_Target *holder_targets(LIBMATTI_MLA_ITransformer *self, size_t *count)
{
    LIBMATTI_ML_TransformerHolder *holder = (LIBMATTI_ML_TransformerHolder *)self;
    return LIBMATTI_MLA_ITransformer_Targets(holder->wrapped, count);
}

static const LIBMATTI_MLA_TargetType *holder_get_target_type(LIBMATTI_MLA_ITransformer *self)
{
    LIBMATTI_ML_TransformerHolder *holder = (LIBMATTI_ML_TransformerHolder *)self;
    return LIBMATTI_MLA_ITransformer_GetTargetType(holder->wrapped);
}

static char **holder_labels(LIBMATTI_MLA_ITransformer *self, size_t *count)
{
    LIBMATTI_ML_TransformerHolder *holder = (LIBMATTI_ML_TransformerHolder *)self;
    return LIBMATTI_MLA_ITransformer_Labels(holder->wrapped, count);
}

LIBMATTI_ML_TransformerHolder *LIBMATTI_ML_TransformerHolder_New(LIBMATTI_MLA_ITransformer *wrapped,
                                                                LIBMATTI_MLA_ITransformationService *owner)
{
    LIBMATTI_ML_TransformerHolder *holder = calloc(1, sizeof(LIBMATTI_ML_TransformerHolder));
    holder->base.transform = holder_transform;
    holder->base.castVote = holder_cast_vote;
    holder->base.targets = holder_targets;
    holder->base.getTargetType = holder_get_target_type;
    holder->base.labels = holder_labels;
    holder->wrapped = wrapped;
    holder->owner = owner;
    return holder;
}

void LIBMATTI_ML_TransformerHolder_Free(LIBMATTI_ML_TransformerHolder *holder)
{
    free(holder);
}

LIBMATTI_MLA_ITransformationService *LIBMATTI_ML_TransformerHolder_Owner(const LIBMATTI_ML_TransformerHolder *holder)
{
    return holder->owner;
}
