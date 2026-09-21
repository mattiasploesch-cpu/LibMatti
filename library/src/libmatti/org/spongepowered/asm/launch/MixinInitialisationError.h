// Port of org.spongepowered.asm.launch.MixinInitialisationError and
// org.spongepowered.asm.mixin.throwables.MixinError (the throwables the loader layer uses).

#ifndef MATTICRAFT_SP_ASM_LAUNCH_MIXININITIALISATIONERROR_H
#define MATTICRAFT_SP_ASM_LAUNCH_MIXININITIALISATIONERROR_H

#include "libmatti/java/lang/Throwable.h"

// Java: public class MixinInitialisationError extends Error
typedef struct
{
    LIBMATTI_JL_Throwable base;
} LIBMATTI_SP_MixinInitialisationError;

// Java: public MixinInitialisationError(String message)
LIBMATTI_SP_MixinInitialisationError *LIBMATTI_SP_MixinInitialisationError_New(const char *message);
void LIBMATTI_SP_MixinInitialisationError_Free(LIBMATTI_SP_MixinInitialisationError *error);

// Java: public class MixinError extends Error
typedef struct
{
    LIBMATTI_JL_Throwable base;
} LIBMATTI_SP_MixinError;

// Java: public MixinError(Throwable cause) / public MixinError(String message)
LIBMATTI_SP_MixinError *LIBMATTI_SP_MixinError_NewWithCause(LIBMATTI_JL_Throwable *cause);
LIBMATTI_SP_MixinError *LIBMATTI_SP_MixinError_New(const char *message);
void LIBMATTI_SP_MixinError_Free(LIBMATTI_SP_MixinError *error);

#endif //MATTICRAFT_SP_ASM_LAUNCH_MIXININITIALISATIONERROR_H
