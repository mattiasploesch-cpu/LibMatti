// Port of net.neoforged.fml.event.lifecycle.FMLCommonSetupEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCOMMONSETUPEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCOMMONSETUPEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent;

// Java: public class FMLCommonSetupEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: FMLCommonSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_Class(void);

// Java: public FMLCommonSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCOMMONSETUPEVENT_H
