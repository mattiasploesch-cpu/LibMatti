// Port of net.neoforged.bus.api.ICancellableEvent.
// Java's default methods mutate Event.isCanceled through the (Event) this cast; the C port passes the
// event itself. The interface type is the class the bus checks an event against
// (ICancellableEvent.class.isAssignableFrom(eventClass)).

#ifndef MATTICRAFT_BUS_API_ICANCELLABLEEVENT_H
#define MATTICRAFT_BUS_API_ICANCELLABLEEVENT_H

#include "libmatti/net/neoforged/bus/api/Event.h"

// Java: ICancellableEvent.class
LIBMATTI_JL_Class *LIBMATTI_BUS_ICancellableEvent_Class(void);

// Java: default void setCanceled(boolean canceled)
void LIBMATTI_BUS_ICancellableEvent_SetCanceled(LIBMATTI_BUS_Event *event, int canceled);
// Java: default boolean isCanceled()
int LIBMATTI_BUS_ICancellableEvent_IsCanceled(const LIBMATTI_BUS_Event *event);

#endif //MATTICRAFT_BUS_API_ICANCELLABLEEVENT_H
