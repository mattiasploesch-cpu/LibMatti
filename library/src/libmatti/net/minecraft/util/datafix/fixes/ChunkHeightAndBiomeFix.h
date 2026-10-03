// Port of net.minecraft.util.datafix.fixes.ChunkHeightAndBiomeFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKHEIGHTANDBIOMEFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKHEIGHTANDBIOMEFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkHeightAndBiomeFix(V2832) - "ChunkHeightAndBiomeFix", CHUNK,
// 2832: the 1.18 world height moved from 0..256 to -64..320, so the 16 stored
// sections become 24 (Y -4..19), every heightmap entry shifts by the 64 block
// offset and the flat biome array becomes a per-section biome container.
//
// Not ported: below_zero_retrogen / blending_data (the bedrock re-generation
// marker) and the UpgradeData index shift - neither is read by the port.
bool LIBMATTI_MC_ChunkHeightAndBiomeFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKHEIGHTANDBIOMEFIX_H