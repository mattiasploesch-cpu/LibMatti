







#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAITEMS_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAITEMS_H

#include <stddef.h>

#include "libmatti/net/minecraft/world/item/Item.h"

// Java: the number of Item statics in Items
#define LIBMATTI_MC_VanillaItems_COUNT 1481

// Java: the Items statics in declaration order (the vanilla registration order)
LIBMATTI_MC_Item **LIBMATTI_MC_VanillaItems_All(void);


LIBMATTI_MC_Item *LIBMATTI_MC_VanillaItems_GetByName(const char *name);


const char *LIBMATTI_MC_VanillaItems_IdOf(LIBMATTI_MC_Item *item);
// Java: Items.AIR - the item every DefaultedMappedRegistry falls back to
LIBMATTI_MC_Item *LIBMATTI_MC_VanillaItems_AIR(void);



void LIBMATTI_MC_VanillaItems_Initialize(void);

// Java: Items.registerAll() - Registry.register(BuiltInRegistries.ITEM, key, item) for
// every static, in declaration order (the RegistryBootstrap the ITEM loader runs)
void LIBMATTI_MC_VanillaItems_RegisterAll(void);

#endif
