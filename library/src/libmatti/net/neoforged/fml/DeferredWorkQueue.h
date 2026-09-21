// Port of net.neoforged.fml.DeferredWorkQueue.
// Java's enqueueWork builds a CompletableFuture whose executor only stores the Runnable, so the work
// runs when runTasks() is called; the port queues the task and runs it there.
// Java's runTasks() throws the aggregate of the failed tasks; the port reports the failures through
// LIBMATTI_FML_DeferredWorkQueue_GetFailures and completes the task futures exceptionally.
// Java's Stopwatch is a monotonic clock here.

#ifndef MATTICRAFT_FML_DEFERREDWORKQUEUE_H
#define MATTICRAFT_FML_DEFERREDWORKQUEUE_H

#include "libmatti/java/util/concurrent/CompletableFuture.h"

#include <stddef.h>

// Java: Runnable / Supplier<T>
typedef void (*LIBMATTI_FML_DeferredWorkQueue_Task)(void *userdata);
typedef void *(*LIBMATTI_FML_DeferredWorkQueue_Supplier)(void *userdata);

typedef struct LIBMATTI_FML_DeferredWorkQueue LIBMATTI_FML_DeferredWorkQueue;
typedef struct LIBMATTI_FML_ModContainer LIBMATTI_FML_ModContainer;

// Java: public DeferredWorkQueue(String name)
LIBMATTI_FML_DeferredWorkQueue *LIBMATTI_FML_DeferredWorkQueue_New(const char *name);
void LIBMATTI_FML_DeferredWorkQueue_Free(LIBMATTI_FML_DeferredWorkQueue *queue);

// Java: public void runTasks() - returns the number of failed tasks (Java throws the aggregate)
int LIBMATTI_FML_DeferredWorkQueue_RunTasks(LIBMATTI_FML_DeferredWorkQueue *queue);

// Java: public CompletableFuture<Void> enqueueWork(ModContainer modInfo, Runnable work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_DeferredWorkQueue_EnqueueWork(
    LIBMATTI_FML_DeferredWorkQueue *queue, LIBMATTI_FML_ModContainer *owner,
    LIBMATTI_FML_DeferredWorkQueue_Task work, void *userdata);
// Java: public <T> CompletableFuture<T> enqueueWork(ModContainer modInfo, Supplier<T> work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_DeferredWorkQueue_EnqueueWorkSupplier(
    LIBMATTI_FML_DeferredWorkQueue *queue, LIBMATTI_FML_ModContainer *owner,
    LIBMATTI_FML_DeferredWorkQueue_Supplier work, void *userdata);

// Java: the catch clause of the task run (captureException) - a C task reports its failure itself
// through this hook while it runs; the future of the failing task completes exceptionally.
void LIBMATTI_FML_DeferredWorkQueue_FailTask(const char *message);
// Java: the aggregate ("modid: message" entries); caller frees the strings and the array
char **LIBMATTI_FML_DeferredWorkQueue_GetFailures(LIBMATTI_FML_DeferredWorkQueue *queue, size_t *count);

#endif //MATTICRAFT_FML_DEFERREDWORKQUEUE_H
