// Port of net.neoforged.fml.event.lifecycle.ParallelDispatchEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

// Java: ParallelDispatchEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.ParallelDispatchEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC | LIBMATTI_JL_Modifier_ABSTRACT);
}

// Java: public ParallelDispatchEvent(ModContainer container, DeferredWorkQueue workQueue)
void LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_ModContainer *container,
    LIBMATTI_FML_DeferredWorkQueue *workQueue)
{
    LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_Init(&event->base, container);
    event->base.base.clazz = LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class();
    event->workQueue = workQueue;
}

// Java: public CompletableFuture<Void> enqueueWork(Runnable work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_EnqueueWork(
    const LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_DeferredWorkQueue_Task work,
    void *userdata)
{
    // Java: return workQueue.enqueueWork(getContainer(), work);
    return LIBMATTI_FML_DeferredWorkQueue_EnqueueWork(event->workQueue,
                                               LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetContainer(&event->base),
                                               work, userdata);
}

// Java: public <T> CompletableFuture<T> enqueueWork(Supplier<T> work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_EnqueueWorkSupplier(
    const LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_DeferredWorkQueue_Supplier work,
    void *userdata)
{
    // Java: return workQueue.enqueueWork(getContainer(), work);
    return LIBMATTI_FML_DeferredWorkQueue_EnqueueWorkSupplier(
        event->workQueue, LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent_GetContainer(&event->base), work, userdata);
}
