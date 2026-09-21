#include "libmatti/net/neoforged/fml/loading/modscan/BackgroundScanHandler.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/util/concurrent/Executor.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

#include <stdlib.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

const char *LIBMATTI_FML_BackgroundScanHandler_StatusName(LIBMATTI_FML_BackgroundScanHandler_ScanStatus status)
{
    switch (status)
    {
    case LIBMATTI_FML_BackgroundScanHandler_NOT_STARTED: return "NOT_STARTED";
    case LIBMATTI_FML_BackgroundScanHandler_RUNNING: return "RUNNING";
    case LIBMATTI_FML_BackgroundScanHandler_COMPLETE: return "COMPLETE";
    case LIBMATTI_FML_BackgroundScanHandler_TIMED_OUT: return "TIMED_OUT";
    case LIBMATTI_FML_BackgroundScanHandler_INTERRUPTED: return "INTERRUPTED";
    case LIBMATTI_FML_BackgroundScanHandler_ERRORED: return "ERRORED";
    }
    return "ERRORED";
}

// Java: private synchronized void logFailure(IModFile file, Throwable throwable)
void LIBMATTI_FML_BackgroundScanHandler_LogFailure(LIBMATTI_FML_BackgroundScanHandler *handler,
                                                   LIBMATTI_NEOFORGESPI_IModFile *file, const char *throwable)
{
    if (throwable == NULL) return;

    handler->status = LIBMATTI_FML_BackgroundScanHandler_ERRORED;
    // Java: LOGGER.error(LogMarkers.SCAN, "An error occurred scanning file {}", file, throwable);
    LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "An error occurred scanning file {}", file,
                             throwable);
}

// Java: public BackgroundScanHandler(Collection<IModFile> modFiles)
LIBMATTI_FML_BackgroundScanHandler *LIBMATTI_FML_BackgroundScanHandler_New(
    LIBMATTI_NEOFORGESPI_IModFile **modFiles, size_t modFileCount)
{
    LIBMATTI_FML_BackgroundScanHandler *handler = calloc(1, sizeof(LIBMATTI_FML_BackgroundScanHandler));

    // Java: int maxThreads = FMLConfig.getIntConfigValue(FMLConfig.ConfigValue.MAX_THREADS);
    //       int poolSize = Math.max(1, maxThreads - 1);
    int maxThreads = LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_MAX_THREADS);
    size_t poolSize = maxThreads - 1 > 1 ? (size_t) (maxThreads - 1) : 1;
    (void) poolSize;

    handler->status = LIBMATTI_FML_BackgroundScanHandler_NOT_STARTED;

    if (handler->status == LIBMATTI_FML_BackgroundScanHandler_RUNNING)
    {
        // Java: if (modContentScanner.isShutdown()) { status = ERRORED; throw new IllegalStateException("Scanner has shutdown"); }
        handler->status = LIBMATTI_FML_BackgroundScanHandler_ERRORED;
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Scanner has shutdown");
        return handler;
    }

    // Java: status = RUNNING; for (var modFile : modFiles) { ((ModFile) modFile).startScan(modContentScanner)
    //         .whenComplete((ignored, t) -> this.logFailure(modFile, t)); }
    // Java: CompletableFuture.supplyAsync(scan, modContentScanner) - the port's queued executor is the
    // ModContentScanner pool stand-in; the direct path keeps the synchronous scan.
    handler->status = LIBMATTI_FML_BackgroundScanHandler_RUNNING;
    LIBMATTI_JUC_Executor *scanner = LIBMATTI_JUC_Executor_QueuedExecutor();
    for (size_t i = 0; i < modFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFile_AsConcrete(modFiles[i]);
        LIBMATTI_JUC_CompletableFuture *future = LIBMATTI_FML_ModFile_StartScan(modFile, scanner);
        // Java: .whenComplete((ignored, t) -> this.logFailure(modFile, t))
        if (future != NULL && LIBMATTI_JUC_CompletableFuture_IsCompletedExceptionally(future))
            LIBMATTI_FML_BackgroundScanHandler_LogFailure(handler, modFiles[i], "scan failed");
    }
    // Java: the pool drained at shutdown - the port drains its queued executor right here
    LIBMATTI_JUC_Executor_DrainQueued(scanner);
    handler->status = LIBMATTI_FML_BackgroundScanHandler_COMPLETE;

    return handler;
}

void LIBMATTI_FML_BackgroundScanHandler_Free(LIBMATTI_FML_BackgroundScanHandler *handler)
{
    free(handler);
}

// Java: public void waitForScanToComplete(Runnable ticker)
int LIBMATTI_FML_BackgroundScanHandler_WaitForScanToComplete(LIBMATTI_FML_BackgroundScanHandler *handler,
                                                             void (*ticker)(void *userdata), void *userdata)
{
    // Java: boolean timeoutActive = System.getProperty("fml.disableScanTimeout") == null;
    //       Instant deadline = Instant.now().plus(Duration.ofMinutes(10));
    //       modContentScanner.shutdown();
    //       do { ticker.run(); status = modContentScanner.awaitTermination(50, MILLISECONDS) ? COMPLETE : RUNNING; ... } while (RUNNING);
    //       if (status == INTERRUPTED) Thread.currentThread().interrupt();
    //       if (status != COMPLETE) throw new IllegalStateException("Failed to complete mod scan");
    // The port has no thread pool, so the scans already completed in the constructor.
    if (ticker != NULL) ticker(userdata);

    if (handler->status != LIBMATTI_FML_BackgroundScanHandler_COMPLETE)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to complete mod scan");
        return 0;
    }

    return 1;
}
