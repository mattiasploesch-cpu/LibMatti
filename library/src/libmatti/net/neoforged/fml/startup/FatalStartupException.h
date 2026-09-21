#ifndef MATTICRAFT_FML_FATALSTARTUPEXCEPTION_H
#define MATTICRAFT_FML_FATALSTARTUPEXCEPTION_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/fml/startup/StartupArgs.h"

// Java: public class FatalStartupException extends RuntimeException
typedef struct
{
    LIBMATTI_JL_Throwable base;
    LIBMATTI_FML_StartupArgs *startupArgs;
} LIBMATTI_FML_FatalStartupException;

// Java: FatalStartupException(String message, StartupArgs startupArgs)
LIBMATTI_FML_FatalStartupException *LIBMATTI_FML_FatalStartupException_New(
    const char *message, LIBMATTI_FML_StartupArgs *startupArgs);
// Java: FatalStartupException(String message, StartupArgs startupArgs, Throwable cause)
LIBMATTI_FML_FatalStartupException *LIBMATTI_FML_FatalStartupException_NewWithCause(
    const char *message, LIBMATTI_FML_StartupArgs *startupArgs, LIBMATTI_JL_Throwable *cause);

void LIBMATTI_FML_FatalStartupException_Free(LIBMATTI_FML_FatalStartupException *exception);

// Java: StartupArgs getStartupArgs()
LIBMATTI_FML_StartupArgs *LIBMATTI_FML_FatalStartupException_GetStartupArgs(
    const LIBMATTI_FML_FatalStartupException *exception);

#endif //MATTICRAFT_FML_FATALSTARTUPEXCEPTION_H
