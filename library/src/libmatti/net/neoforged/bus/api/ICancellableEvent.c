// Port of net.neoforged.bus.api.ICancellableEvent.

#include "libmatti/net/neoforged/bus/api/ICancellableEvent.h"

#include "libmatti/java/lang/reflect/Modifier.h"

// Java: ICancellableEvent.class
LIBMATTI_JL_Class *LIBMATTI_BUS_ICancellableEvent_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.bus.api.ICancellableEvent", NULL, NULL,
                                           LIBMATTI_JL_Modifier_INTERFACE | LIBMATTI_JL_Modifier_ABSTRACT);
}

// Java: ((Event) this).isCanceled = canceled;
void LIBMATTI_BUS_ICancellableEvent_SetCanceled(LIBMATTI_BUS_Event *event, int canceled)
{
    event->isCanceled = canceled;
}

// Java: return ((Event) this).isCanceled;
int LIBMATTI_BUS_ICancellableEvent_IsCanceled(const LIBMATTI_BUS_Event *event)
{
    return event->isCanceled;
}
