// Port of net.minecraft.world.item.Items - the registry-data part.
// Java initialises 1481 Item statics inside registerAll() and registers every one through
// Registry.register(BuiltInRegistries.ITEM, key, item); the C port keeps the vanilla
// declaration order as a static table - VanillaBootstrap registers the whole table through
// the vanilla RegisterEvent, mods read the entries through LIBMATTI_MC_VanillaItems_GetByName.
// The DataComponents part stays the game port's.

#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAITEMS_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAITEMS_H

#include <stddef.h>

#include "libmatti/net/minecraft/world/item/Item.h"

// Java: the number of Item statics in Items
#define LIBMATTI_MC_VanillaItems_COUNT 1481

// Java: the Items statics in declaration order (the vanilla registration order)
LIBMATTI_MC_Item **LIBMATTI_MC_VanillaItems_All(void);
// The port's accessor by Java constant name ("STONE"); NULL when unknown
LIBMATTI_MC_Item *LIBMATTI_MC_VanillaItems_GetByName(const char *name);
// The registered id of the entry ("stone"); NULL when the item is not in the table
const char *LIBMATTI_MC_VanillaItems_IdOf(LIBMATTI_MC_Item *item);
// Java: Items.AIR - the item every DefaultedMappedRegistry falls back to
LIBMATTI_MC_Item *LIBMATTI_MC_VanillaItems_AIR(void);

// The class-initialisation Java's Items static block runs (fills the table)
void LIBMATTI_MC_VanillaItems_Initialize(void);

// Java: Items.registerAll() - Registry.register(BuiltInRegistries.ITEM, key, item) for
// every static, in declaration order (the RegistryBootstrap the ITEM loader runs)
void LIBMATTI_MC_VanillaItems_RegisterAll(void);

#endif //MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAITEMS_H
