// Port of cpw.mods.modlauncher.TransformerVote.

#include "libmatti/cpw/modlauncher/TransformerVote.h"

LIBMATTI_ML_TransformerVote LIBMATTI_ML_TransformerVote_New(LIBMATTI_MLA_TransformerVoteResult result,
                                                            LIBMATTI_MLA_ITransformer *transformer)
{
    LIBMATTI_ML_TransformerVote vote;
    vote.transformer = transformer;
    vote.result = result;
    return vote;
}

LIBMATTI_MLA_TransformerVoteResult LIBMATTI_ML_TransformerVote_GetResult(const LIBMATTI_ML_TransformerVote *vote)
{
    return vote->result;
}

LIBMATTI_MLA_ITransformer *LIBMATTI_ML_TransformerVote_GetTransformer(const LIBMATTI_ML_TransformerVote *vote)
{
    return vote->transformer;
}
