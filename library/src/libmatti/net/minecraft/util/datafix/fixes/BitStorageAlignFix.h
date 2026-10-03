// Port of net.minecraft.util.datafix.fixes.BitStorageAlignFix (P7.3).

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBITSTORAGEALIGNFIX_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBITSTORAGEALIGNFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: new BitStorageAlignFix(V2527) - "BitStorageAlignFix", CHUNK, 2527: the
// 1.18 storage dropped the partial bits at the end of a long, so an old word
// array has to be re-packed entry by entry.
bool LIBMATTI_MC_BitStorageAlignFix_Apply(LIBMATTI_MC_Nbt_Tag *root);

// Java: public static long[] addPadding(int size, int bits, long[] data) -
// re-packs `size` entries of `bits` width out of the old (gap-carrying) words.
// The caller owns the returned array.
int64_t *LIBMATTI_MC_BitStorageAlignFix_AddPadding(int size, int bits, const int64_t *data, size_t dataLength,
                                                   size_t *outLength);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBITSTORAGEALIGNFIX_H