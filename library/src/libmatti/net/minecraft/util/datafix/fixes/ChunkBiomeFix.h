// Port of net.minecraft.util.datafix.fixes.ChunkBiomeFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKBIOMEFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKBIOMEFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkBiomeFix(V2202, false) - "Leaves fix", CHUNK, 2202: the
// 1.15 chunks stored one biome id per 4x4 corner (256 ids); the rule expands
// them into the 16x16 grid the 1.16 storage expects.
bool LIBMATTI_MC_ChunkBiomeFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKBIOMEFIX_H