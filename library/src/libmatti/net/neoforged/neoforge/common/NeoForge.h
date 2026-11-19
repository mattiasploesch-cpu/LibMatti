




#ifndef MATTICRAFT_NEOFORGE_COMMON_NEOFORGE_H
#define MATTICRAFT_NEOFORGE_COMMON_NEOFORGE_H

#include "libmatti/net/neoforged/bus/api/IEventBus.h"

// Java: public static final IEventBus EVENT_BUS - the "game" bus, built once on first use
LIBMATTI_BUS_IEventBus *LIBMATTI_NEOFORGE_NeoForge_EVENT_BUS(void);

#endif
