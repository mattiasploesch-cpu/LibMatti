// Port of net.neoforged.fml.event.lifecycle.InterModProcessEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/InterModProcessEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: InterModProcessEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.InterModProcessEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public InterModProcessEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent_Free(LIBMATTI_FML_Event_Lifecycle_InterModProcessEvent *event)
{
    free(event);
}
