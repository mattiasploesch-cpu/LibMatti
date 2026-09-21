// Port of cpw.mods.modlauncher.TransformerHolder.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMERHOLDER_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMERHOLDER_H

#include "libmatti/cpw/modlauncher/api/ITransformer.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"

// Java: public class TransformerHolder<T> implements ITransformer<T>
typedef struct LIBMATTI_ML_TransformerHolder
{
    // Java: implements ITransformer<T>
    LIBMATTI_MLA_ITransformer base;
    LIBMATTI_MLA_ITransformer *wrapped;
    LIBMATTI_MLA_ITransformationService *owner;
} LIBMATTI_ML_TransformerHolder;

// Java: public TransformerHolder(ITransformer<T> wrapped, ITransformationService owner)
LIBMATTI_ML_TransformerHolder *LIBMATTI_ML_TransformerHolder_New(LIBMATTI_MLA_ITransformer *wrapped,
                                                                LIBMATTI_MLA_ITransformationService *owner);
void LIBMATTI_ML_TransformerHolder_Free(LIBMATTI_ML_TransformerHolder *holder);

// Java: ITransformationService owner()
LIBMATTI_MLA_ITransformationService *LIBMATTI_ML_TransformerHolder_Owner(const LIBMATTI_ML_TransformerHolder *holder);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMERHOLDER_H
