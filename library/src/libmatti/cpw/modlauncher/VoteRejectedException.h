// Port of cpw.mods.modlauncher.VoteRejectedException.

#ifndef MATTICRAFT_MODLAUNCHER_VOTEREJECTEDEXCEPTION_H
#define MATTICRAFT_MODLAUNCHER_VOTEREJECTEDEXCEPTION_H

#include "libmatti/cpw/modlauncher/TransformerVote.h"

#include <stddef.h>

// Java: public class VoteRejectedException extends RuntimeException
typedef struct
{
    LIBMATTI_ML_TransformerVote *votes;
    size_t voteCount;
    const char *nodeType;
} LIBMATTI_ML_VoteRejectedException;

#endif //MATTICRAFT_MODLAUNCHER_VOTEREJECTEDEXCEPTION_H
