// Port of net.neoforged.neoforgespi.locating.ModFileLoadingException.
// TODO: java.lang.RuntimeException - the port keeps the Java exception as a log line.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_MODFILELOADINGEXCEPTION_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_MODFILELOADINGEXCEPTION_H

#include "libmatti/java/lang/Throwable.h"

// Java: public ModFileLoadingException(String message)
LIBMATTI_JL_Throwable *LIBMATTI_NEOFORGESPI_ModFileLoadingException_New(const char *message);
// Java: public ModFileLoadingException(String message, Throwable cause)
LIBMATTI_JL_Throwable *LIBMATTI_NEOFORGESPI_ModFileLoadingException_NewWithCause(const char *message,
                                                                                LIBMATTI_JL_Throwable *cause);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_MODFILELOADINGEXCEPTION_H
