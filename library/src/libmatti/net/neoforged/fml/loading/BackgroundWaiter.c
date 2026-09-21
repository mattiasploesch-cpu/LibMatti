// Port of net.neoforged.fml.loading.BackgroundWaiter.

#include "libmatti/net/neoforged/fml/loading/BackgroundWaiter.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

typedef struct
{
    LIBMATTI_FML_BackgroundWaiter_Task task;
    void *userdata;
    atomic_int done;
} Work;

static void *work_main(void *arg)
{
    Work *work = arg;
    work->task(work->userdata);
    atomic_store(&work->done, 1);
    return NULL;
}

// Java: public static void runAndTick(Runnable r, Runnable tick)
void LIBMATTI_FML_BackgroundWaiter_RunAndTick(LIBMATTI_FML_BackgroundWaiter_Task r, void *rUserdata,
                                             LIBMATTI_FML_BackgroundWaiter_Task tick, void *tickUserdata)
{
    LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress("Loading bootstrap resources");

    Work *work = calloc(1, sizeof(Work));
    work->task = r;
    work->userdata = rUserdata;

    pthread_t handle;
    if (pthread_create(&handle, NULL, work_main, work) != 0)
    {
        // Java: the executor could not accept the work
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to submit the background work");
        free(work);
        return;
    }

    do
    {
        tick(tickUserdata);
        LIBMATTI_JL_Thread_Sleep(50);
    }
    while (!atomic_load(&work->done));

    // Java: runner.shutdown(); work.get();
    pthread_join(handle, NULL);
    free(work);
}
