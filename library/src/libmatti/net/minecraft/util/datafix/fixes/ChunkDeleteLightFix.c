// Port of net.minecraft.util.datafix.fixes.ChunkDeleteLightFix (P7.3).
//
// Java: remove("isLightOn") on the root plus, through
// type.findField("sections"), remove("BlockLight").remove("SkyLight") on the
// section list.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkDeleteLightFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

bool LIBMATTI_MC_ChunkDeleteLightFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *root = (LIBMATTI_MC_Nbt_CompoundTag *) rootTag;

    bool changed = false;
    if (LIBMATTI_MC_Nbt_CompoundTag_Contains(root, "isLightOn"))
    {
        LIBMATTI_MC_Nbt_CompoundTag_Remove(root, "isLightOn");
        changed = true;
    }

    // Java: the light arrays live on the root and on every section; the 1.18+
    // sections carry BlockLight/SkyLight, the old flat ones did not
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf(root);
    static const char *const LIGHT_KEYS[] = {"BlockLight", "SkyLight"};
    for (size_t k = 0; k < sizeof(LIGHT_KEYS) / sizeof(LIGHT_KEYS[0]); k++)
    {
        if (level != NULL && LIBMATTI_MC_Nbt_CompoundTag_Contains(level, LIGHT_KEYS[k]))
        {
            LIBMATTI_MC_Nbt_CompoundTag_Remove(level, LIGHT_KEYS[k]);
            changed = true;
        }
    }

    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_FixChunks_SectionsOf(root);
    if (sections != NULL)
    {
        int count = LIBMATTI_MC_Nbt_ListTag_Size(sections);
        for (int i = 0; i < count; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *section = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, i, &section) || section == NULL)
                continue;
            for (size_t k = 0; k < sizeof(LIGHT_KEYS) / sizeof(LIGHT_KEYS[0]); k++)
                if (LIBMATTI_MC_Nbt_CompoundTag_Contains(section, LIGHT_KEYS[k]))
                {
                    LIBMATTI_MC_Nbt_CompoundTag_Remove(section, LIGHT_KEYS[k]);
                    changed = true;
                }
        }
    }
    return changed;
}