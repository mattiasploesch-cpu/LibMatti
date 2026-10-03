// Port of net.minecraft.util.datafix.fixes.ChunkToProtochunkFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKTOPROTOCHUNKFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKTOPROTOCHUNKFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkToProtochunkFix(V1466, true) - "ChunkToProtoChunkFix", CHUNK,
// 1466: the terrain/light flags become a ChunkStatus string and the tile tick
// list becomes the packed post-processing lists.
bool LIBMATTI_MC_ChunkToProtochunkFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKTOPROTOCHUNKFIX_H