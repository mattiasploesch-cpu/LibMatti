// Port of net.minecraft.util.datafix.fixes.ChunkBiomeFix (P7.3).
//
// Java reads the int array "Biomes" (256 entries), picks each 4x4 corner's
// value, stamps that value over the 4x4 block it covers, then repeats the
// first 16-entry row 64 times to fill the 1024-entry biome container.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkBiomeFix.h"

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <stdlib.h>
#include <string.h>

bool LIBMATTI_MC_ChunkBiomeFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: p_145206_.get("Biomes").asIntStreamOpt() - the pre-1.16 biome id
    // array; a chunk without it (or with the wrong length) is left alone
    const int32_t *oldBiomes = NULL;
    size_t oldLength = 0;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(level, "Biomes", &oldBiomes, &oldLength))
        return false;
    if (oldLength != 256)
        return false;

    int32_t expanded[1024];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int k = (j << 2) + 2; // the corner offset inside its 4x4 block
            int l = (i << 2) + 2;
            int index = l << 4 | k;
            expanded[i << 2 | j] = oldBiomes[index];
        }
    }
    // Java: System.arraycopy(aint1, 0, aint1, j1 * 16, 16) for j1 in 1..63
    for (int row = 1; row < 64; row++)
        memcpy(expanded + row * 16, expanded, 16 * sizeof(int32_t));

    LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(level, "Biomes", expanded, 1024);
    return true;
}