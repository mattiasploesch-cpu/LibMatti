#include "libmatti/net/neoforged/fml/startup/FatalStartupException.h"

#include <stdlib.h>

static const char *FATAL_STARTUP_EXCEPTION_NAME = "net.neoforged.fml.startup.FatalStartupException";

static LIBMATTI_FML_FatalStartupException *fatal_startup_exception_new(const char *message,
                                                                      LIBMATTI_FML_StartupArgs *startupArgs,
                                                                      LIBMATTI_JL_Throwable *cause)
{
    LIBMATTI_FML_FatalStartupException *exception = calloc(1, sizeof(LIBMATTI_FML_FatalStartupException));

    LIBMATTI_JL_Throwable *base = LIBMATTI_JL_Throwable_NewNamed(FATAL_STARTUP_EXCEPTION_NAME, message);
    exception->base = *base;
    free(base);

    if (cause != NULL) LIBMATTI_JL_Throwable_SetCause(&exception->base, cause);

    exception->startupArgs = startupArgs;
    return exception;
}

LIBMATTI_FML_FatalStartupException *LIBMATTI_FML_FatalStartupException_New(
    const char *message, LIBMATTI_FML_StartupArgs *startupArgs)
{
    return fatal_startup_exception_new(message, startupArgs, NULL);
}

LIBMATTI_FML_FatalStartupException *LIBMATTI_FML_FatalStartupException_NewWithCause(
    const char *message, LIBMATTI_FML_StartupArgs *startupArgs, LIBMATTI_JL_Throwable *cause)
{
    return fatal_startup_exception_new(message, startupArgs, cause);
}

void LIBMATTI_FML_FatalStartupException_Free(LIBMATTI_FML_FatalStartupException *exception)
{
    free(exception);
}

LIBMATTI_FML_StartupArgs *LIBMATTI_FML_FatalStartupException_GetStartupArgs(
    const LIBMATTI_FML_FatalStartupException *exception)
{
    return exception->startupArgs;
}
