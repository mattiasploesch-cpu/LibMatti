// Port of net.neoforged.fml.event.lifecycle.FMLLoadCompleteEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_FMLLOADCOMPLETEEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_FMLLOADCOMPLETEEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent;

// Java: public class FMLLoadCompleteEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: FMLLoadCompleteEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_Class(void);

// Java: public FMLLoadCompleteEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_FMLLOADCOMPLETEEVENT_H
