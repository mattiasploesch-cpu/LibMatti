





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

#endif
