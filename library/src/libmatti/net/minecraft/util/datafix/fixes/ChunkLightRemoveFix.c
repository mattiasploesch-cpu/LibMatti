// Port of net.minecraft.util.datafix.fixes.ChunkLightRemoveFix (P7.3).
//
// Java: fieldFinder("Level") + remainderFinder().remove("isLightOn") - the
// 1.17 chunks carried a light flag that the rewriter derives on load.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkLightRemoveFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

bool LIBMATTI_MC_ChunkLightRemoveFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *root = (LIBMATTI_MC_Nbt_CompoundTag *) rootTag;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf(root);
    if (level == NULL || !LIBMATTI_MC_Nbt_CompoundTag_Contains(level, "isLightOn"))
        return false;
    LIBMATTI_MC_Nbt_CompoundTag_Remove(level, "isLightOn");
    return true;
}