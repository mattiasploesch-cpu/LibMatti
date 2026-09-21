#include "libmatti/net/neoforged/neoforgespi/locating/InvalidModFileException.h"

#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public InvalidModFileException(String message, IModFileInfo modFileInfo)
LIBMATTI_NEOFORGESPI_InvalidModFileException *LIBMATTI_NEOFORGESPI_InvalidModFileException_New(const char *message,
                                                                                             void *modFileInfo)
{
    // Java: "%s (%s)" with Optional.ofNullable(modFileInfo).map(mf -> mf.getFile().getFileName()).orElse("MISSING FILE NAME")
    const char *fileName = "MISSING FILE NAME";
    if (modFileInfo != NULL)
    {
        LIBMATTI_NEOFORGESPI_IModFile *file =
            LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile((LIBMATTI_NEOFORGESPI_IModFileInfo *)modFileInfo);
        if (file != NULL)
            fileName = LIBMATTI_NEOFORGESPI_IModFile_GetFileName(file);
    }

    size_t length = strlen(message) + strlen(fileName) + 4;
    char *fullMessage = malloc(length);
    snprintf(fullMessage, length, "%s (%s)", message, fileName);

    LIBMATTI_NEOFORGESPI_InvalidModFileException *exception = calloc(1, sizeof(*exception));
    exception->base = *LIBMATTI_NEOFORGESPI_ModFileLoadingException_New(fullMessage);
    free(fullMessage);
    exception->modFileInfo = modFileInfo;
    return exception;
}

// Java: public IModFileInfo getBrokenFile()
void *LIBMATTI_NEOFORGESPI_InvalidModFileException_GetBrokenFile(
    const LIBMATTI_NEOFORGESPI_InvalidModFileException *exception)
{
    return exception->modFileInfo;
}
