// Port of net.neoforged.fml.event.lifecycle.InterModProcessEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODPROCESSEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODPROCESSEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent;

// Java: public class InterModProcessEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: InterModProcessEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_Class(void);

// Java: public InterModProcessEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_Free(LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_INTERMODPROCESSEVENT_H
