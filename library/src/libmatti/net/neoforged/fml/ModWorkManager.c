// Port of net.neoforged.fml.ModWorkManager.

#include "libmatti/net/neoforged/fml/ModWorkManager.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/loading/FMLConfig.h"

#include <stdio.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: Executors.newSingleThreadExecutor(...) runs the tasks in submission order on one thread;
// the C port lets the caller's thread do that, which preserves that order.
static void run_on_caller(void *self, LIBMATTI_FML_ModWorkManager_Task task, void *userdata)
{
    (void)self;
    task(userdata);
}

// Java: public static Executor syncExecutor()
LIBMATTI_FML_ModWorkManager_Executor LIBMATTI_FML_ModWorkManager_SyncExecutor(void)
{
    LIBMATTI_FML_ModWorkManager_Executor executor = {NULL, run_on_caller};
    return executor;
}

// Java: public static Executor parallelExecutor()
LIBMATTI_FML_ModWorkManager_Executor LIBMATTI_FML_ModWorkManager_ParallelExecutor(void)
{
    // Java: LOGGER.debug(LOADING, "Using {} threads for parallel mod-loading", loadingThreadCount);
    char threadCount[32];
    snprintf(threadCount, sizeof(threadCount), "%d",
             LIBMATTI_FML_FMLConfig_GetIntConfigValue(LIBMATTI_FML_FMLConfig_MAX_THREADS));
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Using {} threads for parallel mod-loading",
                             threadCount);

    LIBMATTI_FML_ModWorkManager_Executor executor = {NULL, run_on_caller};
    return executor;
}

// Java: void execute(Runnable command)
void LIBMATTI_FML_ModWorkManager_Executor_Execute(const LIBMATTI_FML_ModWorkManager_Executor *executor,
                                                  LIBMATTI_FML_ModWorkManager_Task task, void *userdata)
{
    executor->execute(executor->self, task, userdata);
}
