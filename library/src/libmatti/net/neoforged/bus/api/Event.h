// Port of net.neoforged.bus.api.Event.
// Java: public abstract class Event { boolean isCanceled = false; }
// Every event carries its Class because the bus looks listeners up by the event's runtime type
// (Java: Object.getClass()).

#ifndef MATTICRAFT_BUS_API_EVENT_H
#define MATTICRAFT_BUS_API_EVENT_H

#include "libmatti/java/lang/Class.h"

typedef struct LIBMATTI_BUS_Event LIBMATTI_BUS_Event;

struct LIBMATTI_BUS_Event
{
    // Java: Object.getClass()
    LIBMATTI_JL_Class *clazz;
    // Java: boolean isCanceled = false;
    int isCanceled;
};

// Java: the subclass constructor runs super() - the C port sets the runtime type there
void LIBMATTI_BUS_Event_Init(LIBMATTI_BUS_Event *event, LIBMATTI_JL_Class *clazz);

// Java: Event.class
LIBMATTI_JL_Class *LIBMATTI_BUS_Event_Class(void);

#endif //MATTICRAFT_BUS_API_EVENT_H
