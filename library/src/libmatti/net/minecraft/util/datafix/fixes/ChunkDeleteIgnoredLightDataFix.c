// Port of net.minecraft.util.datafix.fixes.ChunkDeleteIgnoredLightDataFix (P7.3).
//
// Java: the light arrays only survive when the chunk is still flagged lit -
//   boolean flag = get(remainderFinder()).get("isLightOn").asBoolean(false);
//   return !flag ? updateTyped(sections, remove("BlockLight").remove("SkyLight")) : input

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkDeleteIgnoredLightDataFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

bool LIBMATTI_MC_ChunkDeleteIgnoredLightDataFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *root = (LIBMATTI_MC_Nbt_CompoundTag *) rootTag;

    // Java: the flag is read off the chunk root, not through the Level optic
    int isLightOn = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(root, "isLightOn", 0);
    if (isLightOn)
        return false; // Java: the rule leaves a lit chunk alone

    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_FixChunks_SectionsOf(root);
    if (sections == NULL)
        return false;

    bool changed = false;
    int count = LIBMATTI_MC_Nbt_ListTag_Size(sections);
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *section = NULL;
        if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, i, &section) || section == NULL)
            continue;
        if (LIBMATTI_MC_Nbt_CompoundTag_Contains(section, "BlockLight"))
        {
            LIBMATTI_MC_Nbt_CompoundTag_Remove(section, "BlockLight");
            changed = true;
        }
        if (LIBMATTI_MC_Nbt_CompoundTag_Contains(section, "SkyLight"))
        {
            LIBMATTI_MC_Nbt_CompoundTag_Remove(section, "SkyLight");
            changed = true;
        }
    }
    return changed;
}