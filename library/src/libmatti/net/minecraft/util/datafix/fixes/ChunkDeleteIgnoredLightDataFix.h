// Port of net.minecraft.util.datafix.fixes.ChunkDeleteIgnoredLightDataFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETEIGNOREDLIGHTDATAFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETEIGNOREDLIGHTDATAFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkDeleteIgnoredLightDataFix(V3077, true) - "ChunkDeleteIgnored-
// LightDataFix", CHUNK, 3077
bool LIBMATTI_MC_ChunkDeleteIgnoredLightDataFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETEIGNOREDLIGHTDATAFIX_H