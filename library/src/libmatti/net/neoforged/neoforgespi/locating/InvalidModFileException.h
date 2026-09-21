// Port of net.neoforged.neoforgespi.locating.InvalidModFileException.
// TODO: net.neoforged.neoforgespi.language.IModFileInfo - wired as an opaque handle.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_INVALIDMODFILEEXCEPTION_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_INVALIDMODFILEEXCEPTION_H

#include "libmatti/net/neoforged/neoforgespi/locating/ModFileLoadingException.h"

// Java: public class InvalidModFileException extends ModFileLoadingException
typedef struct LIBMATTI_NEOFORGESPI_InvalidModFileException
{
    LIBMATTI_JL_Throwable base;
    // Java: private final IModFileInfo modFileInfo
    void *modFileInfo;
} LIBMATTI_NEOFORGESPI_InvalidModFileException;

// Java: public InvalidModFileException(String message, IModFileInfo modFileInfo)
LIBMATTI_NEOFORGESPI_InvalidModFileException *LIBMATTI_NEOFORGESPI_InvalidModFileException_New(const char *message,
                                                                                             void *modFileInfo);

// Java: public IModFileInfo getBrokenFile()
void *LIBMATTI_NEOFORGESPI_InvalidModFileException_GetBrokenFile(
    const LIBMATTI_NEOFORGESPI_InvalidModFileException *exception);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_INVALIDMODFILEEXCEPTION_H
