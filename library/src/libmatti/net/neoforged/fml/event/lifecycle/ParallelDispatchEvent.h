// Port of net.neoforged.fml.event.lifecycle.ParallelDispatchEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_PARALLELDISPATCHEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_PARALLELDISPATCHEVENT_H

#include "libmatti/net/neoforged/fml/DeferredWorkQueue.h"
#include "libmatti/net/neoforged/fml/event/lifecycle/ModLifecycleEvent.h"

// Java: public abstract class ParallelDispatchEvent extends ModLifecycleEvent
typedef struct LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent;

struct LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent
{
    // Java: extends ModLifecycleEvent
    LIBMATTI_FML_Event_Lifecycle_ModLifecycleEvent base;
    // Java: private final DeferredWorkQueue workQueue;
    LIBMATTI_FML_DeferredWorkQueue *workQueue;
};

// Java: ParallelDispatchEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(void);

// Java: public ParallelDispatchEvent(ModContainer container, DeferredWorkQueue workQueue)
void LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_ModContainer *container,
    LIBMATTI_FML_DeferredWorkQueue *workQueue);

// Java: public CompletableFuture<Void> enqueueWork(Runnable work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_EnqueueWork(
    const LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_DeferredWorkQueue_Task work,
    void *userdata);
// Java: public <T> CompletableFuture<T> enqueueWork(Supplier<T> work)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_EnqueueWorkSupplier(
    const LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent *event, LIBMATTI_FML_DeferredWorkQueue_Supplier work,
    void *userdata);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_PARALLELDISPATCHEVENT_H
