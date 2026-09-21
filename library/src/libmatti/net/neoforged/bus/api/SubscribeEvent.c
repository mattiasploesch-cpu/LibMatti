// Port of net.neoforged.bus.api.SubscribeEvent.

#include "libmatti/net/neoforged/bus/api/SubscribeEvent.h"

LIBMATTI_BUS_SubscribeEvent LIBMATTI_BUS_SubscribeEvent_Default(void)
{
    LIBMATTI_BUS_SubscribeEvent event = {LIBMATTI_BUS_EventPriority_NORMAL, 0};
    return event;
}
