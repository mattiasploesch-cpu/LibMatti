// Port of net.neoforged.bus.api.SubscribeEvent.
// Java reads the annotation from a method through reflection (@Retention(RUNTIME) @Target(METHOD));
// C has neither annotations nor reflection, so the port passes the annotation's values as a plain value.
// Java defaults: priority = EventPriority.NORMAL, receiveCanceled = false.

#ifndef MATTICRAFT_BUS_API_SUBSCRIBEEVENT_H
#define MATTICRAFT_BUS_API_SUBSCRIBEEVENT_H

#include "libmatti/net/neoforged/bus/api/EventPriority.h"

// Java: public @interface SubscribeEvent
typedef struct
{
    // Java: EventPriority priority() default EventPriority.NORMAL;
    LIBMATTI_BUS_EventPriority priority;
    // Java: boolean receiveCanceled() default false;
    int receiveCanceled;
} LIBMATTI_BUS_SubscribeEvent;

// Java: the annotation defaults
LIBMATTI_BUS_SubscribeEvent LIBMATTI_BUS_SubscribeEvent_Default(void);

#endif //MATTICRAFT_BUS_API_SUBSCRIBEEVENT_H
