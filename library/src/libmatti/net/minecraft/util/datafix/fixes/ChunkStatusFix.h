// Port of net.minecraft.util.datafix.fixes.ChunkStatusFix and
// ChunkStatusFix2 (P7.3) - both rewrite the chunk status string, and both are
// registered on the same field, so the port keeps two applies to mirror the
// two rules (1905 "postprocessed" -> "fullchunk", 1911 "fullchunk" ->
// "full").

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKSTATUSFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKSTATUSFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkStatusFix(V1905, false) - "ChunkStatusFix"
bool LIBMATTI_MC_ChunkStatusFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

// Java: new ChunkStatusFix2(V1911, false) - "ChunkStatusFix2"
bool LIBMATTI_MC_ChunkStatusFix2_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKSTATUSFIX_H