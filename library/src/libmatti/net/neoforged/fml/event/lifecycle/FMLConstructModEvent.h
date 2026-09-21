// Port of net.neoforged.fml.event.lifecycle.FMLConstructModEvent.

#ifndef MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCONSTRUCTMODEVENT_H
#define MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCONSTRUCTMODEVENT_H

#include "libmatti/net/neoforged/fml/event/lifecycle/ParallelDispatchEvent.h"

typedef struct LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent;

// Java: public class FMLConstructModEvent extends ParallelDispatchEvent
struct LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent
{
    // Java: extends ParallelDispatchEvent
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent base;
};

// Java: FMLConstructModEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Class(void);

// Java: public FMLConstructModEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue);
void LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Free(
    LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *event);

#endif //MATTICRAFT_FML_EVENT_LIFECYCLE_FMLCONSTRUCTMODEVENT_H
