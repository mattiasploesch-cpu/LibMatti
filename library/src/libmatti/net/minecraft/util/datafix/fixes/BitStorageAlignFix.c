// Port of net.minecraft.util.datafix.fixes.BitStorageAlignFix (P7.3).
//
// Java: updateHeightmaps + updateSections + addPadding - the 1.18 storage reads
// exactly floor(64 / bits) entries per long, so the pre-1.18 words (which left
// the spare bits empty per word) have to be re-packed entry by entry.

#include "libmatti/net/minecraft/util/datafix/fixes/BitStorageAlignFix.h"
#include "libmatti/com/mojang/datafixers/DataFixUtils.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <stdlib.h>

int64_t *LIBMATTI_MC_BitStorageAlignFix_AddPadding(int size, int bits, const int64_t *data, size_t dataLength,
                                                   size_t *outLength)
{
    if (outLength != NULL)
        *outLength = 0;
    // Java: int i = p_14740_.length; if (i == 0) return p_14740_;
    if (dataLength == 0 || bits <= 0 || bits > 32)
        return NULL;

    const int perLong = 64 / bits;
    const size_t needed = ((size_t) size + (size_t) perLong - 1) / (size_t) perLong;
    int64_t *out = calloc(needed, sizeof(int64_t));
    if (out == NULL)
        return NULL;

    const uint64_t mask = (bits == 64) ? UINT64_MAX : (((uint64_t) 1 << bits) - 1);

    // Java reads the old words through a two-word sliding window, refilling as
    // the entry's bit offset crosses a word boundary
    int outIndex = 0;
    int outBits = 0;
    uint64_t outWord = 0;
    int inWord = 0;      // the word index the entry starts in
    uint64_t low = (uint64_t) data[0];
    uint64_t high = dataLength > 1 ? (uint64_t) data[1] : 0;

    for (int k = 0; k < size; k++)
    {
        int bitOffset = k * bits;
        int wordIndex = bitOffset >> 6;
        int endWordIndex = ((k + 1) * bits - 1) >> 6;
        int shift = bitOffset ^ (wordIndex << 6);

        if (wordIndex != inWord)
        {
            low = high;
            high = wordIndex + 1 < (int) dataLength ? (uint64_t) data[wordIndex + 1] : 0;
            inWord = wordIndex;
        }

        uint64_t value;
        if (wordIndex == endWordIndex)
            value = (low >> shift) & mask;
        else
            value = ((low >> shift) | (high << (64 - shift))) & mask;

        int next = outBits + bits;
        if (next >= 64)
        {
            out[outIndex++] = (int64_t) outWord;
            outWord = value;
            outBits = bits;
        }
        else
        {
            outWord |= value << outBits;
            outBits = next;
        }
    }

    // Java: if (k1 != 0L) along[i1] = k1; - a fully written word stays zero
    if (outWord != 0 && (size_t) outIndex < needed)
        out[outIndex] = (int64_t) outWord;

    if (outLength != NULL)
        *outLength = needed;
    return out;
}

// Java: updateBitStorage(Dynamic parent, Dynamic storage, int size, int bits)
static bool repack(LIBMATTI_MC_Nbt_CompoundTag *parent, const char *key, int size, int bits)
{
    const int64_t *raw = NULL;
    size_t rawLength = 0;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(parent, key, &raw, &rawLength))
        return false;
    size_t outLength = 0;
    int64_t *padded = LIBMATTI_MC_BitStorageAlignFix_AddPadding(size, bits, raw, rawLength, &outLength);
    if (padded == NULL)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(parent, key, padded, outLength);
    free(padded);
    return true;
}

bool LIBMATTI_MC_BitStorageAlignFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *root = (LIBMATTI_MC_Nbt_CompoundTag *) rootTag;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf(root);
    if (level == NULL)
        return false;

    bool changed = false;

    // Java: updateSections - the palette size gives the entry width, and only
    // a width that is not a power of two needs a re-pack
    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_FixChunks_SectionsOf(root);
    if (sections != NULL)
    {
        int count = LIBMATTI_MC_Nbt_ListTag_Size(sections);
        for (int i = 0; i < count; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *section = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, i, &section) || section == NULL)
                continue;
            LIBMATTI_MC_Nbt_ListTag *palette = NULL;
            if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(section, "Palette", &palette) || palette == NULL)
                continue;
            int bits = LIBMATTI_MC_DataFixUtils_CeilLog2(LIBMATTI_MC_Nbt_ListTag_Size(palette));
            if (bits < 4)
                bits = 4;
            // Java: i != 0 && !Mth.isPowerOfTwo(i)
            if (bits == 0 || (bits & (bits - 1)) == 0)
                continue;
            if (repack(section, "BlockStates", 4096, bits))
                changed = true;
        }
    }

    // Java: updateHeightmaps - 256 entries at 9 bits each
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetCompound(level, "Heightmaps", &heightmaps) && heightmaps != NULL)
    {
        size_t keyCount = 0;
        // Java: updateMapValues over the heightmap map - KeySet hands out
        // the compound's internal array, which the caller must not free
        char **keys = LIBMATTI_MC_Nbt_CompoundTag_KeySet(heightmaps, &keyCount);
        for (size_t i = 0; i < keyCount; i++)
            if (repack(heightmaps, keys[i], 256, 9))
                changed = true;
    }

    return changed;
}