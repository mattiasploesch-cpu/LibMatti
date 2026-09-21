// Port of net.neoforged.fml.event.lifecycle.InterModEnqueueEvent.

#include "libmatti/net/neoforged/fml/event/lifecycle/InterModEnqueueEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>

// Java: InterModEnqueueEvent.class
LIBMATTI_JL_Class *LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.fml.event.lifecycle.InterModEnqueueEvent", NULL,
                                           LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Class(),
                                           LIBMATTI_JL_Modifier_PUBLIC);
}

// Java: public InterModEnqueueEvent(ModContainer container, DeferredWorkQueue deferredWorkQueue)
LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_New(
    LIBMATTI_FML_ModContainer *container, LIBMATTI_FML_DeferredWorkQueue *deferredWorkQueue)
{
    LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *event =
        calloc(1, sizeof(LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent));
    LIBMATTI_FML_Event_Lifecycle_ParallelDispatchEvent_Init(&event->base, container, deferredWorkQueue);
    event->base.base.base.clazz = LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_Class();
    return event;
}

void LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent_Free(LIBMATTI_FML_Event_Lifecycle_InterModEnqueueEvent *event)
{
    free(event);
}
