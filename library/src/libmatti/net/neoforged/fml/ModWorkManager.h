// Port of net.neoforged.fml.ModWorkManager.
// Java: java.util.concurrent.Executor; the sync executor is the game thread (direct), the parallel
// executor is Java's ForkJoinPool.commonPool() - the port's queued executor with an immediate drain
// keeps the submission order the loader relies on.

#ifndef MATTICRAFT_FML_MODWORKMANAGER_H
#define MATTICRAFT_FML_MODWORKMANAGER_H

// Java: Runnable
typedef void (*LIBMATTI_FML_ModWorkManager_Task)(void *userdata);

// Java: public interface Executor { void execute(Runnable command); }
typedef struct
{
    void *self;
    void (*execute)(void *self, LIBMATTI_FML_ModWorkManager_Task task, void *userdata);
} LIBMATTI_FML_ModWorkManager_Executor;

// Java: public static Executor syncExecutor()
LIBMATTI_FML_ModWorkManager_Executor LIBMATTI_FML_ModWorkManager_SyncExecutor(void);
// Java: public static Executor parallelExecutor()
LIBMATTI_FML_ModWorkManager_Executor LIBMATTI_FML_ModWorkManager_ParallelExecutor(void);

// Java: void execute(Runnable command)
void LIBMATTI_FML_ModWorkManager_Executor_Execute(const LIBMATTI_FML_ModWorkManager_Executor *executor,
                                                  LIBMATTI_FML_ModWorkManager_Task task, void *userdata);

#endif //MATTICRAFT_FML_MODWORKMANAGER_H
