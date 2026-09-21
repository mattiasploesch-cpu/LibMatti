// Port of net.neoforged.fml.event.lifecycle.FMLCommonSetupEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/FMLCommonSetupEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: FMLCommonSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.FMLCommonSetupEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public FMLCommonSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent_Free(LIBMATTI_FML_Event_Lifecycle_FMLCommonSetupEvent *event)
{
    free(event);
}
