// Port of net.neoforged.fml.event.lifecycle.FMLConstructModEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/FMLConstructModEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: FMLConstructModEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.FMLConstructModEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public FMLConstructModEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent_Free(
    LIBMATTI_FML_Event_Lifecycle_FMLConstructModEvent *event)
{
    free(event);
}
