// Port of net.neoforged.neoforge.common.NeoForge.
// Only the class's statics the loader path touches are ported; the game-content side of NeoForge
// (its registry entries, common handlers) is the game port's part.

#ifndef MATTICRAFT_NEOFORGE_COMMON_NEOFORGE_H
#define MATTICRAFT_NEOFORGE_COMMON_NEOFORGE_H

#include "libmatti/net/neoforged/bus/api/IEventBus.h"

// Java: public static final IEventBus EVENT_BUS - the "game" bus, built once on first use
LIBMATTI_BUS_IEventBus *LIBMATTI_NEOFORGE_NeoForge_EVENT_BUS(void);

#endif //MATTICRAFT_NEOFORGE_COMMON_NEOFORGE_H
