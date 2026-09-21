// Port of net.neoforged.fml.event.lifecycle.FMLDedicatedServerSetupEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/FMLDedicatedServerSetupEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: FMLDedicatedServerSetupEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.FMLDedicatedServerSetupEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public FMLDedicatedServerSetupEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *
LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_New(LIBMATTI_FML_ModContainer *container,
                                                             LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent_Free(
    LIBMATTI_FML_Event_Lifecycle_FMLDedicatedServerSetupEvent *event)
{
    free(event);
}
