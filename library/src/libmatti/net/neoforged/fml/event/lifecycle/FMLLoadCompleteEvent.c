// Port of net.neoforged.fml.event.lifecycle.FMLLoadCompleteEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/FMLLoadCompleteEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: FMLLoadCompleteEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.FMLLoadCompleteEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public FMLLoadCompleteEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLLoadCompleteEvent *event)
{
    free(event);
}
