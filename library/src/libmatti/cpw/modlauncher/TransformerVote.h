// Port of cpw.mods.modlauncher.TransformerVote.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMERVOTE_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMERVOTE_H

#include "libmatti/cpw/modlauncher/api/ITransformer.h"

// Java: class TransformerVote<T>
typedef struct
{
    LIBMATTI_MLA_ITransformer *transformer;
    LIBMATTI_MLA_TransformerVoteResult result;
} LIBMATTI_ML_TransformerVote;

// Java: TransformerVote(TransformerVoteResult vr, ITransformer<T> transformer)
LIBMATTI_ML_TransformerVote LIBMATTI_ML_TransformerVote_New(LIBMATTI_MLA_TransformerVoteResult result,
                                                            LIBMATTI_MLA_ITransformer *transformer);

// Java: TransformerVoteResult getResult()
LIBMATTI_MLA_TransformerVoteResult LIBMATTI_ML_TransformerVote_GetResult(const LIBMATTI_ML_TransformerVote *vote);
// Java: ITransformer<T> getTransformer()
LIBMATTI_MLA_ITransformer *LIBMATTI_ML_TransformerVote_GetTransformer(const LIBMATTI_ML_TransformerVote *vote);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMERVOTE_H
