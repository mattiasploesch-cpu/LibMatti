#include "libmatti/net/neoforged/fml/startup/StartupArgs.h"

#include <stdlib.h>

void LIBMATTI_FML_StartupArgs_Free(LIBMATTI_FML_StartupArgs *startupArgs)
{
    if (startupArgs == NULL) return;

    free(startupArgs->gameDirectory);

    for (size_t i = 0; i < startupArgs->claimedFileCount; i++)
        free(startupArgs->claimedFiles[i]);
    free(startupArgs->claimedFiles);

    for (size_t i = 0; i < startupArgs->unclaimedClassPathEntryCount; i++)
        free(startupArgs->unclaimedClassPathEntries[i]);
    free(startupArgs->unclaimedClassPathEntries);

    free(startupArgs);
}
