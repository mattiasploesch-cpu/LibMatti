// Port of net.neoforged.fml.event.lifecycle.FMLClientSetupEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/FMLClientSetupEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: FMLClientSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.FMLClientSetupEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public FMLClientSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLClientSetupEvent *event)
{
    free(event);
}
