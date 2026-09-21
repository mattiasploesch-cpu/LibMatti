// Port of net.neoforged.bus.api.EventPriority.

#ifndef MATTICRAFT_BUS_API_EVENTPRIORITY_H
#define MATTICRAFT_BUS_API_EVENTPRIORITY_H

#include <stddef.h>

// Java: public enum EventPriority { HIGHEST, HIGH, NORMAL, LOW, LOWEST }
// The declaration order is the ordinal order; the ListenerList indexes its lists by it.
typedef enum
{
    LIBMATTI_BUS_EventPriority_HIGHEST, //First to execute
    LIBMATTI_BUS_EventPriority_HIGH,
    LIBMATTI_BUS_EventPriority_NORMAL,
    LIBMATTI_BUS_EventPriority_LOW,
    LIBMATTI_BUS_EventPriority_LOWEST //Last to execute
} LIBMATTI_BUS_EventPriority;

// Java: EventPriority.values().length
#define LIBMATTI_BUS_EventPriority_COUNT 5

// Java: public int ordinal()
int LIBMATTI_BUS_EventPriority_Ordinal(LIBMATTI_BUS_EventPriority priority);
// Java: public String name()
const char *LIBMATTI_BUS_EventPriority_Name(LIBMATTI_BUS_EventPriority priority);

#endif //MATTICRAFT_BUS_API_EVENTPRIORITY_H
