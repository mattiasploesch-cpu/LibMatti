// Port of net.neoforged.fml.DeferredWorkQueue.

#include "libmatti/net/neoforged/fml/DeferredWorkQueue.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/ModLoadingContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static class TaskInfo { ModContainer owner; Runnable task; CompletableFuture<?> future; }
typedef struct
{
    LIBMATTI_FML_ModContainer *owner;
    LIBMATTI_FML_DeferredWorkQueue_Task task;
    void *userdata;
    // Java: CompletableFuture<?> future - the future enqueueWork returned
    LIBMATTI_JUC_CompletableFuture *future;
} TaskInfo;

struct LIBMATTI_FML_DeferredWorkQueue
{
    // Java: private final ConcurrentLinkedDeque<TaskInfo> tasks
    TaskInfo *tasks;
    size_t taskCount;
    char *name;
    // Java: RuntimeException aggregate of the failed tasks
    char **failures;
    size_t failureCount;
    // Java: the TaskInfo the running task belongs to (for FailTask from inside the task)
    TaskInfo *running;
};

// Java: Stopwatch timer = Stopwatch.createStarted()
static double timer_seconds(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec + (double)now.tv_nsec / 1e9;
}

// Java: public DeferredWorkQueue(String name)
LIBMATTI_FML_DeferredWorkQueue *LIBMATTI_FML_DeferredWorkQueue_New(const char *name)
{
    LIBMATTI_FML_DeferredWorkQueue *queue = calloc(1, sizeof(LIBMATTI_FML_DeferredWorkQueue));
    queue->name = strdup(name);
    return queue;
}

void LIBMATTI_FML_DeferredWorkQueue_Free(LIBMATTI_FML_DeferredWorkQueue *queue)
{
    if (queue == NULL) return;
    for (size_t i = 0; i < queue->failureCount; i++) free(queue->failures[i]);
    free(queue->failures);
    free(queue->tasks);
    free(queue->name);
    free(queue);
}

// Java: the task runs on the thread that called runTasks(); the port keeps the running queue in a
// file-static (the loader transforms and dispatches on one thread), which is the ThreadLocal behaviour.
static LIBMATTI_FML_DeferredWorkQueue *runningQueue = NULL;

// Java: public void enqueueWork(ModContainer modInfo, Runnable work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_DeferredWorkQueue_EnqueueWork(
    LIBMATTI_FML_DeferredWorkQueue *queue, LIBMATTI_FML_ModContainer *owner,
    LIBMATTI_FML_DeferredWorkQueue_Task work, void *userdata)
{
    queue->tasks = realloc(queue->tasks, sizeof(*queue->tasks) * (queue->taskCount + 1));
    queue->tasks[queue->taskCount].owner = owner;
    queue->tasks[queue->taskCount].task = work;
    queue->tasks[queue->taskCount].userdata = userdata;
    // Java: CompletableFuture.runAsync(work, r -> taskInfo.task = r) - the executor stores the runnable
    // on the TaskInfo, so the future completes when runTasks() runs it
    queue->tasks[queue->taskCount].future = LIBMATTI_JUC_CompletableFuture_New();
    return queue->tasks[queue->taskCount++].future;
}

// Java: public <T> CompletableFuture<T> enqueueWork(ModContainer modInfo, Supplier<T> work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_DeferredWorkQueue_EnqueueWorkSupplier(
    LIBMATTI_FML_DeferredWorkQueue *queue, LIBMATTI_FML_ModContainer *owner,
    LIBMATTI_FML_DeferredWorkQueue_Supplier work, void *userdata)
{
    // Java: CompletableFuture.supplyAsync(work, r -> taskInfo.task = r); the port's supplier adapts
    // to the task signature (the value would only be observable through the future)
    return LIBMATTI_FML_DeferredWorkQueue_EnqueueWork(queue, owner, (LIBMATTI_FML_DeferredWorkQueue_Task)work,
                                                      userdata);
}

// Java: captureException - the catch clause around ti.task.run(); a C task calls FailTask from
// inside its body to report the failure the Java catch clause would have caught.
void LIBMATTI_FML_DeferredWorkQueue_FailTask(const char *message)
{
    if (runningQueue == NULL || runningQueue->running == NULL) return;

    TaskInfo *task = runningQueue->running;
    const char *modId = task->owner != NULL ? task->owner->modId : "unknown";

    // Java: captureException(modId, aggregate, throwable) adds "modId: message" to the aggregate
    size_t size = strlen(modId) + strlen(message) + 8;
    char *failure = malloc(size);
    snprintf(failure, size, "%s: %s", modId, message);
    runningQueue->failures = realloc(runningQueue->failures,
                                     sizeof(*runningQueue->failures) * (runningQueue->failureCount + 1));
    runningQueue->failures[runningQueue->failureCount++] = failure;

    // Java: the task's future completes exceptionally (CompletionException wrapping the cause)
    LIBMATTI_JUC_CompletableFuture_CompleteExceptionally(task->future, NULL);

    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Mod '{}' encountered an error in a deferred task: {}", modId, message);
}

// Java: the aggregate ("modid: message" entries) of the last runTasks() call
char **LIBMATTI_FML_DeferredWorkQueue_GetFailures(LIBMATTI_FML_DeferredWorkQueue *queue, size_t *count)
{
    *count = queue->failureCount;
    return queue->failures;
}

// Java: private static void makeRunnable(TaskInfo ti, Executor executor, RuntimeException aggregate)
static void make_runnable(LIBMATTI_FML_DeferredWorkQueue *queue, TaskInfo *task)
{
    double start = timer_seconds();

    // Java: ModLoadingContext.get().setActiveContainer(ti.owner);
    LIBMATTI_FML_ModLoadingContext *context = LIBMATTI_FML_ModLoadingContext_Get();
    LIBMATTI_FML_ModLoadingContext_SetActiveContainer(context, task->owner);

    // Java: try { ti.task.run(); } catch (Throwable t) { captureException(...) } - a C task cannot
    // throw, so it completes the future exceptionally through FailTask (NULL message = success)
    queue->running = task;
    runningQueue = queue;
    task->task(task->userdata);
    runningQueue = NULL;
    queue->running = NULL;
    // NULL message = success: the future completes normally (a failing task called FailTask,
    // which already completed it exceptionally)
    LIBMATTI_JUC_CompletableFuture_Complete(task->future, NULL);

    LIBMATTI_FML_ModLoadingContext_SetActiveContainer(context, NULL);

    // Java: if (timer.elapsed(TimeUnit.SECONDS) >= 1) LOGGER.warn(LOADING, "Mod '{}' took {} to run a deferred task.", ...)
    double elapsed = timer_seconds() - start;
    if (elapsed >= 1)
    {
        char seconds[32];
        snprintf(seconds, sizeof(seconds), "%.3fs", elapsed);
        LIBMATTI_ML_Logger_Warn(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                "Mod '{}' took {} to run a deferred task.", task->owner->modId, seconds);
    }
}

// Java: public void runTasks()
int LIBMATTI_FML_DeferredWorkQueue_RunTasks(LIBMATTI_FML_DeferredWorkQueue *queue)
{
    if (queue->taskCount == 0) return 0;

    // Java: LOGGER.debug(LOADING, "Dispatching synchronous work for work queue {}: {} jobs", name, tasks.size());
    char jobCount[32];
    snprintf(jobCount, sizeof(jobCount), "%zu", queue->taskCount);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                             "Dispatching synchronous work for work queue {}: {} jobs", queue->name, jobCount);

    double start = timer_seconds();
    for (size_t i = 0; i < queue->taskCount; i++)
        make_runnable(queue, &queue->tasks[i]);
    double elapsed = timer_seconds() - start;

    // Java: runTasks threw the aggregate - the failed futures carry it now

    if (queue->failureCount > 0)
    {
        // Java: LOGGER.fatal(LOADING, "Synchronous work queue completed exceptionally in {}, see suppressed
        //       exceptions for details:", timer, aggregate); throw aggregate;
        char seconds[32];
        snprintf(seconds, sizeof(seconds), "%.3fs", elapsed);
        LIBMATTI_ML_Logger_Fatal(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Synchronous work queue completed exceptionally in {}: {}", seconds,
                                 queue->failures[0]);
        return (int)queue->failureCount;
    }

    // Java: LOGGER.debug(LOADING, "Synchronous work queue completed in {}", timer);
    char seconds[32];
    snprintf(seconds, sizeof(seconds), "%.3fs", elapsed);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Synchronous work queue completed in {}",
                             seconds);
    return 0;
}
