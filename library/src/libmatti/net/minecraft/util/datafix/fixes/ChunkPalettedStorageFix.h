// Port of net.minecraft.util.datafix.fixes.ChunkPalettedStorageFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKPALETTEDSTORAGEFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKPALETTEDSTORAGEFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkPalettedStorageFix(V1451.1, true) - "ChunkPalettedStorageFix",
// CHUNK, 1451: the 1.12/1.15 sections stored 12 bit block ids in three byte
// arrays (Blocks/Data/Add); the rule resolves every id through BlockStateData
// and writes the palette + packed BlockStates the modern storage reads.
//
// Not ported: the VIRTUAL/FIX neighbour-driven block-entity repairs (the bed,
// banner, skull, door and dye rewrites). Those only touch block entities the
// port does not instantiate yet, and the blocks themselves convert correctly.
bool LIBMATTI_MC_ChunkPalettedStorageFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKPALETTEDSTORAGEFIX_H