// Port of net.neoforged.bus.api.EventPriority.

#include "libmatti/net/neoforged/bus/api/EventPriority.h"

// Java: public int ordinal()
int LIBMATTI_BUS_EventPriority_Ordinal(LIBMATTI_BUS_EventPriority priority)
{
    return (int)priority;
}

// Java: public String name()
const char *LIBMATTI_BUS_EventPriority_Name(LIBMATTI_BUS_EventPriority priority)
{
    static const char *names[LIBMATTI_BUS_EventPriority_COUNT] = {"HIGHEST", "HIGH", "NORMAL", "LOW", "LOWEST"};
    return names[priority];
}
