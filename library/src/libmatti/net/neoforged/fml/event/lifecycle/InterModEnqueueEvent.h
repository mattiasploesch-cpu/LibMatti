// Port of net.neoforged.fml.event.lifecycle.InterModEnqueueEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODENQUEUEEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODENQUEUEEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent;

// Java: public class InterModEnqueueEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: InterModEnqueueEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_Class(void);

// Java: public InterModEnqueueEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_Free(LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODENQUEUEEVENT_H
