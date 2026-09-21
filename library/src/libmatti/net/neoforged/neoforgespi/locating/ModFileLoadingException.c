#include "libmatti/net/neoforged/neoforgespi/locating/ModFileLoadingException.h"

// Java: public ModFileLoadingException(String message)
LIBMATTI_JL_Throwable *LIBMATTI_NEOFORGESPI_ModFileLoadingException_New(const char *message)
{
    return LIBMATTI_JL_Throwable_NewNamed("net.neoforged.neoforgespi.locating.ModFileLoadingException", message);
}

// Java: public ModFileLoadingException(String message, Throwable cause)
LIBMATTI_JL_Throwable *LIBMATTI_NEOFORGESPI_ModFileLoadingException_NewWithCause(const char *message,
                                                                                LIBMATTI_JL_Throwable *cause)
{
    LIBMATTI_JL_Throwable *throwable =
        LIBMATTI_JL_Throwable_NewNamed("net.neoforged.neoforgespi.locating.ModFileLoadingException", message);
    throwable->cause = cause;
    return throwable;
}
