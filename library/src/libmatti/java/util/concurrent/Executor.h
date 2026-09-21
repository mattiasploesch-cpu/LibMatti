// Port of java.util.concurrent.Executor / ExecutorService.
// The C port has no real threads by default: the "direct" executor runs the
// command inline (Java: Executors.newDirectExecutorService behaviour), the
// thread-pool executor is a FIFO queue the host drains.

#ifndef MATTICRAFT_JAVA_UTIL_CONCURRENT_EXECUTOR_H
#define MATTICRAFT_JAVA_UTIL_CONCURRENT_EXECUTOR_H

#include <stddef.h>

// Java: interface Executor - void execute(Runnable command)
typedef void (*LIBMATTI_JUC_Runnable)(void *self);

typedef struct LIBMATTI_JUC_Executor LIBMATTI_JUC_Executor;

struct LIBMATTI_JUC_Executor
{
    // Java: void execute(Runnable command)
    void (*execute)(LIBMATTI_JUC_Executor *executor, LIBMATTI_JUC_Runnable command, void *self);
    // the queued executor's FIFO storage (the direct executor keeps these empty)
    int queued;
    void **commands;
    size_t count;
    size_t capacity;
};

// Java: the caller-runs executor every loader path uses when single threaded
LIBMATTI_JUC_Executor *LIBMATTI_JUC_Executor_DirectExecutor(void);

// Java: a queued executor; commands are stored and drained later by the host
LIBMATTI_JUC_Executor *LIBMATTI_JUC_Executor_QueuedExecutor(void);
// Runs every queued command FIFO (the port's "thread pool drain"); returns the number run
size_t LIBMATTI_JUC_Executor_DrainQueued(LIBMATTI_JUC_Executor *executor);
// Java: public boolean isTerminated() for the queued executor
int LIBMATTI_JUC_Executor_IsQueuedEmpty(const LIBMATTI_JUC_Executor *executor);

#endif //MATTICRAFT_JAVA_UTIL_CONCURRENT_EXECUTOR_H
