// Port of net.minecraft.util.datafix.fixes.ChunkDeleteLightFix (P7.3).
//
// The rule is registered twice in DataFixers.addFixers - once for the 1.20
// schema (3451) and once for the 1.21.4 schema (4537) - with the same body and
// only the rule name differing, so the port exposes one apply and both
// registry entries.

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETELIGHTFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETELIGHTFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new ChunkDeleteLightFix(V3451, false) and new ChunkDeleteLightFix(
// V4537, false) - "ChunkDeleteLightFix for 3451" / "for 4537", CHUNK
bool LIBMATTI_MC_ChunkDeleteLightFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKDELETELIGHTFIX_H