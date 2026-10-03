// Port of net.minecraft.util.datafix.fixes.ChunkLightRemoveFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKLIGHTREMOVEFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKLIGHTREMOVEFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkLightRemoveFix(V1961, false) - "ChunkLightRemoveFix", CHUNK,
// 1961
bool LIBMATTI_MC_ChunkLightRemoveFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKLIGHTREMOVEFIX_H