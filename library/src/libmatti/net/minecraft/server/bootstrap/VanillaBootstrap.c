// Port of the vanilla content bootstrap (the RegistryBootstrap bodies that fill BLOCK and
// ITEM). Java's createContents() runs the per-registry loaders - the BLOCK loader runs
// Blocks.registerAll(), the ITEM loader Items.registerAll(), and both register every static
// through Registry.register into the BuiltInRegistries; the port mirrors exactly that.

#include "libmatti/net/minecraft/server/bootstrap/VanillaBootstrap.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/inventory/MenuType.h"

// Java: the Blocks/Items/EntityType/MenuType class initialisations (registerAll) the loaders run
void LIBMATTI_MC_VanillaBootstrap_RegisterVanillaContent(void)
{
    LIBMATTI_MC_VanillaBlocks_RegisterAll();
    LIBMATTI_MC_VanillaItems_RegisterAll();
    LIBMATTI_MC_VanillaEntities_RegisterAll();
    LIBMATTI_MC_VanillaMenus_RegisterAll();
}
