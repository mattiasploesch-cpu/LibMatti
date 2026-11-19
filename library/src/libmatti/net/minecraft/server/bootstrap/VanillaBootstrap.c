





#include "libmatti/net/minecraft/server/bootstrap/VanillaBootstrap.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"

// Java: the Blocks/Items/EntityType class initialisations (registerAll) the loaders run
void LIBMATTI_MC_VanillaBootstrap_RegisterVanillaContent(void)
{
    LIBMATTI_MC_VanillaBlocks_RegisterAll();
    LIBMATTI_MC_VanillaItems_RegisterAll();
    LIBMATTI_MC_VanillaEntities_RegisterAll();
}
