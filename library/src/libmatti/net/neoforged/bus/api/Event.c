// Port of net.neoforged.bus.api.Event.

#include "libmatti/net/neoforged/bus/api/Event.h"

#include "libmatti/java/lang/reflect/Modifier.h"

void LIBMATTI_BUS_Event_Init(LIBMATTI_BUS_Event *event, LIBMATTI_JL_Class *clazz)
{
    event->clazz = clazz;
    event->isCanceled = 0;
}

// Java: Event.class - every other event derives from it
LIBMATTI_JL_Class *LIBMATTI_BUS_Event_Class(void)
{
    return LIBMATTI_JL_Class_RegisterClass("net.neoforged.bus.api.Event", NULL, LIBMATTI_JL_Class_ObjectType(),
                                           LIBMATTI_JL_Modifier_PUBLIC | LIBMATTI_JL_Modifier_ABSTRACT);
}
