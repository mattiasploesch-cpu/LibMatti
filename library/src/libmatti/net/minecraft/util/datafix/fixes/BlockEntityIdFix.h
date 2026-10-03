// Port of net.minecraft.util.datafix.fixes.BlockEntityIdFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKENTITYIDFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKENTITYIDFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new BlockEntityIdFix(V704, true) - "BlockEntityIdFix", BLOCK_ENTITY,
// 704. Java rewrites the tagged choice's key (and the item-stack name hook);
// the port walks the chunk's block-entity list instead of the schema tree.
bool LIBMATTI_MC_BlockEntityIdFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

// Java: public static final Map<String, String> ID_MAP - the 1.12 block entity
// names to their modern ids ("Chest" -> "minecraft:chest", ...)
const char *LIBMATTI_MC_BlockEntityIdFix_MapId(const char *id);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKENTITYIDFIX_H