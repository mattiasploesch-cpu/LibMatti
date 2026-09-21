// Port of org.spongepowered.asm.launch.MixinInitialisationError and
// org.spongepowered.asm.mixin.throwables.MixinError.

#include "libmatti/org/spongepowered/asm/launch/MixinInitialisationError.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_SP_MixinInitialisationError *LIBMATTI_SP_MixinInitialisationError_New(const char *message)
{
    LIBMATTI_SP_MixinInitialisationError *error = calloc(1, sizeof(*error));
    // Java: org.spongepowered.asm.launch.MixinInitialisationError
    error->base = *LIBMATTI_JL_Throwable_NewNamed("org.spongepowered.asm.launch.MixinInitialisationError", message);
    return error;
}

void LIBMATTI_SP_MixinInitialisationError_Free(LIBMATTI_SP_MixinInitialisationError *error)
{
    if (error == NULL) return;
    LIBMATTI_JL_Throwable_Free(&error->base);
    free(error);
}

LIBMATTI_SP_MixinError *LIBMATTI_SP_MixinError_NewWithCause(LIBMATTI_JL_Throwable *cause)
{
    LIBMATTI_SP_MixinError *error = calloc(1, sizeof(*error));
    error->base = *LIBMATTI_JL_Throwable_NewNamed("org.spongepowered.asm.mixin.throwables.MixinError", NULL);
    LIBMATTI_JL_Throwable_SetCause(&error->base, cause);
    return error;
}

LIBMATTI_SP_MixinError *LIBMATTI_SP_MixinError_New(const char *message)
{
    LIBMATTI_SP_MixinError *error = calloc(1, sizeof(*error));
    error->base = *LIBMATTI_JL_Throwable_NewNamed("org.spongepowered.asm.mixin.throwables.MixinError", message);
    return error;
}

void LIBMATTI_SP_MixinError_Free(LIBMATTI_SP_MixinError *error)
{
    if (error == NULL) return;
    LIBMATTI_JL_Throwable_Free(&error->base);
    free(error);
}
