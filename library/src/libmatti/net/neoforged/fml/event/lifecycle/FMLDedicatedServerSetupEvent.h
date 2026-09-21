// Port of net.neoforged.fml.event.lifecycle.FMLDedicatedServerSetupEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_FMLDEDICATEDSERVERSETUPEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_FMLDEDICATEDSERVERSETUPEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent;

// Java: public class FMLDedicatedServerSetupEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: FMLDedicatedServerSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_Class(void);

// Java: public FMLDedicatedServerSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *
LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_New(LIBMATTI_FML_ModContainer *container,
                                                             LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_Free(
    LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_FMLDEDICATEDSERVERSETUPEVENT_H
