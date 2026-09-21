// Port of net.neoforged.fml.event.lifecycle.FMLClientSetupEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCLIENTSETUPEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCLIENTSETUPEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent;

// Java: public class FMLClientSetupEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: FMLClientSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_Class(void);

// Java: public FMLClientSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCLIENTSETUPEVENT_H
