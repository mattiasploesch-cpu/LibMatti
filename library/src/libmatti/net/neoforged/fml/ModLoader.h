// Port of net.neoforged.fml.ModLoader.
// Java's executors map to the direct executor (every dispatch runs on the calling thread, which
// keeps the loader's ordering) and the CompletableFuture port carries the task results; failures
// surface through addLoadingIssue, which is also what Java's exception channels feed.
// ModLoadingIssue.Severity comes from the exceptions: isMeaningfulException /
// addLoadingIssuesFromException report through the Throwable port.

#ifndef MATTICRAFT_FML_MODLOADER_H
#define MATTICRAFT_FML_MODLOADER_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/fml/DeferredWorkQueue.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

#include <stddef.h>

// Java: private static final List<ModLoadingIssue> loadingIssues = new ArrayList<>();
void LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue *issue);
// Java: public static void addLoadingIssues(Collection<ModLoadingIssue> issues)
void LIBMATTI_FML_ModLoader_AddLoadingIssues(LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount);

// Java: public static List<ModLoadingIssue> getLoadingIssues()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetLoadingIssues(size_t *count);
// Java: public static List<ModLoadingIssue> getErrors()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetErrors(size_t *count);
// Java: public static List<ModLoadingIssue> getWarnings()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_GetWarnings(size_t *count);
// Java: public static boolean hasErrors()
int LIBMATTI_FML_ModLoader_HasErrors(void);

// Java: public static void clearLoadingIssues()
void LIBMATTI_FML_ModLoader_ClearLoadingIssues(void);
// The port's channel for "an inner operation threw ModLoadingException and the caller
// caught it": takes every pending issue out of the list (the caller owns them), leaving
// the list empty. Java's catch (ModLoadingException e) e.getIssues() is the counterpart.
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoader_TakeLoadingIssues(size_t *count);
// Java: public static void clear()
void LIBMATTI_FML_ModLoader_Clear(void);

// Java: throw new ModLoadingException(loadingIssues) - the port keeps the exception for the
// startup's fatal error report (see FMLLoader.getLastException).
LIBMATTI_JL_Throwable *LIBMATTI_FML_ModLoader_GetLastException(void);

// Java: public static void gatherAndInitializeMods(Executor syncExecutor, Executor parallelExecutor, Runnable periodicTask)
void LIBMATTI_FML_ModLoader_GatherAndInitializeMods(void (*periodicTask)(void *userdata), void *userdata);

// Java: public static void runInitTask(String name, Executor syncExecutor, Runnable periodicTask, Runnable initTask)
void LIBMATTI_FML_ModLoader_RunInitTask(const char *name, void (*periodicTask)(void *userdata), void *userdata,
                                        void (*initTask)(void *userdata), void *taskUserdata);

// Java: public static void dispatchParallelEvent(String name, Executor syncExecutor, Executor parallelExecutor,
//         Runnable periodicTask, BiFunction<ModContainer, DeferredWorkQueue, ParallelDispatchEvent> eventConstructor)
void LIBMATTI_FML_ModLoader_DispatchParallelEvent(
    const char *name, void (*periodicTask)(void *userdata), void *userdata,
    LIBMATTI_BUS_Event *(*eventConstructor)(LIBMATTI_FML_ModContainer *container,
                                            LIBMATTI_FML_DeferredWorkQueue *workQueue, void *userdata),
    void *constructorUserdata);

// Java: public static void waitForTask(String name, Runnable periodicTask, CompletableFuture<?> future)
void LIBMATTI_FML_ModLoader_WaitForTask(const char *name, void (*periodicTask)(void *userdata), void *userdata,
                                        void (*task)(void *userdata), void *taskUserdata);

// Java: public static void dispatchParallelTask(String name, Executor parallelExecutor, Runnable periodicTask,
//         Consumer<ModContainer> task)
void LIBMATTI_FML_ModLoader_DispatchParallelTask(const char *name, void (*periodicTask)(void *userdata),
                                                 void *userdata,
                                                 void (*task)(LIBMATTI_FML_ModContainer *container,
                                                              void *userdata),
                                                 void *taskUserdata);

// Java: public static <T extends Event & IModBusEvent> void runEventGenerator(Function<ModContainer, T> generator)
void LIBMATTI_FML_ModLoader_RunEventGenerator(
    LIBMATTI_BUS_Event *(*generator)(LIBMATTI_FML_ModContainer *container, void *userdata), void *userdata);

// Java: public static <T extends Event & IModBusEvent> void postEvent(T e)
void LIBMATTI_FML_ModLoader_PostEvent(LIBMATTI_BUS_Event *event);
// Java: public static <T extends Event & IModBusEvent> T postEventWithReturn(T e)
LIBMATTI_BUS_Event *LIBMATTI_FML_ModLoader_PostEventWithReturn(LIBMATTI_BUS_Event *event);
// Java: public static <T extends Event & IModBusEvent> void postEventWrapContainerInModOrder(T event)
void LIBMATTI_FML_ModLoader_PostEventWrapContainerInModOrder(LIBMATTI_BUS_Event *event);
// Java: public static <T extends Event & IModBusEvent> void postEventWithWrapInModOrder(T e,
//         BiConsumer<ModContainer, T> pre, BiConsumer<ModContainer, T> post)
void LIBMATTI_FML_ModLoader_PostEventWithWrapInModOrder(
    LIBMATTI_BUS_Event *event,
    void (*pre)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata),
    void (*post)(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event, void *userdata), void *userdata);

#endif //MATTICRAFT_FML_MODLOADER_H
