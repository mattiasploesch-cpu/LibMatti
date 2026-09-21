// Port of cpw.mods.modlauncher.VotingContext.

#ifndef MATTICRAFT_MODLAUNCHER_VOTINGCONTEXT_H
#define MATTICRAFT_MODLAUNCHER_VOTINGCONTEXT_H

#include "libmatti/cpw/modlauncher/api/ITransformerVotingContext.h"

#include <stddef.h>

// Java: class VotingContext implements ITransformerVotingContext
struct LIBMATTI_MLA_ITransformerVotingContext
{
    char *className;
    int classExists;
    // Java: Supplier<byte[]> sha256
    unsigned char *(*sha256)(void *userdata, size_t *length);
    void *sha256Userdata;
    LIBMATTI_MLA_ITransformerActivity *auditActivities;
    size_t auditActivityCount;
    char *reason;
    // Java: Object node
    void *node;
};

typedef LIBMATTI_MLA_ITransformerVotingContext LIBMATTI_ML_VotingContext;

// Java: VotingContext(String className, boolean classExists, Supplier<byte[]> sha256sum,
//                     List<ITransformerActivity> activities, String reason)
LIBMATTI_ML_VotingContext *LIBMATTI_ML_VotingContext_New(const char *className, int classExists,
                                                         unsigned char *(*sha256)(void *userdata, size_t *length),
                                                         void *sha256Userdata,
                                                         LIBMATTI_MLA_ITransformerActivity *activities,
                                                         size_t activityCount, const char *reason);
void LIBMATTI_ML_VotingContext_Free(LIBMATTI_ML_VotingContext *context);

// Java: <T> void setNode(T node)
void LIBMATTI_ML_VotingContext_SetNode(LIBMATTI_ML_VotingContext *context, void *node);

#endif //MATTICRAFT_MODLAUNCHER_VOTINGCONTEXT_H
