// Port of net.neoforged.fml.loading.modscan.BackgroundScanHandler.
// Java's ModContentScanner is a fixed pool; the port uses the queued java.util.concurrent executor
// (commands drain right after submission, which for the loader is the same result).

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_BACKGROUNDSCANHANDLER_H
#define MATTICRAFT_FML_LOADING_MODSCAN_BACKGROUNDSCANHANDLER_H

#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stddef.h>

// Java: private enum ScanStatus
typedef enum
{
    LIBMATTI_FML_BackgroundScanHandler_NOT_STARTED,
    LIBMATTI_FML_BackgroundScanHandler_RUNNING,
    LIBMATTI_FML_BackgroundScanHandler_COMPLETE,
    LIBMATTI_FML_BackgroundScanHandler_TIMED_OUT,
    LIBMATTI_FML_BackgroundScanHandler_INTERRUPTED,
    LIBMATTI_FML_BackgroundScanHandler_ERRORED
} LIBMATTI_FML_BackgroundScanHandler_ScanStatus;

// Java: public class BackgroundScanHandler
typedef struct LIBMATTI_FML_BackgroundScanHandler LIBMATTI_FML_BackgroundScanHandler;

struct LIBMATTI_FML_BackgroundScanHandler
{
    LIBMATTI_FML_BackgroundScanHandler_ScanStatus status;
};

// Java: public BackgroundScanHandler(Collection<IModFile> modFiles)
LIBMATTI_FML_BackgroundScanHandler *LIBMATTI_FML_BackgroundScanHandler_New(
    LIBMATTI_NEOFORGESPI_IModFile **modFiles, size_t modFileCount);
void LIBMATTI_FML_BackgroundScanHandler_Free(LIBMATTI_FML_BackgroundScanHandler *handler);

// Java: private synchronized void logFailure(IModFile file, Throwable throwable)
void LIBMATTI_FML_BackgroundScanHandler_LogFailure(LIBMATTI_FML_BackgroundScanHandler *handler,
                                                   LIBMATTI_NEOFORGESPI_IModFile *file, const char *throwable);

// Java: public void waitForScanToComplete(Runnable ticker) - 1 when the scan completed
int LIBMATTI_FML_BackgroundScanHandler_WaitForScanToComplete(LIBMATTI_FML_BackgroundScanHandler *handler,
                                                             void (*ticker)(void *userdata), void *userdata);

// Java: the enum name, used for logging
const char *LIBMATTI_FML_BackgroundScanHandler_StatusName(LIBMATTI_FML_BackgroundScanHandler_ScanStatus status);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_BACKGROUNDSCANHANDLER_H
