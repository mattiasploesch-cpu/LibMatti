// Port of cpw.mods.modlauncher.VoteDeadlockException.

#ifndef MATTICRAFT_MODLAUNCHER_VOTEDEADLOCKEXCEPTION_H
#define MATTICRAFT_MODLAUNCHER_VOTEDEADLOCKEXCEPTION_H

#include "libmatti/cpw/modlauncher/TransformerVote.h"

#include <stddef.h>

// Java: public class VoteDeadlockException extends RuntimeException
typedef struct
{
    LIBMATTI_ML_TransformerVote *votes;
    size_t voteCount;
    const char *nodeType;
} LIBMATTI_ML_VoteDeadlockException;

#endif //MATTICRAFT_MODLAUNCHER_VOTEDEADLOCKEXCEPTION_H
