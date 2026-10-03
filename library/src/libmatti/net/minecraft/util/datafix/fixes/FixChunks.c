// The chunk fixers' optic-path helper (P7.3).

#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_FixChunks_LevelOf(LIBMATTI_MC_Nbt_CompoundTag *root)
{
    if (root == NULL)
        return NULL;
    LIBMATTI_MC_Nbt_CompoundTag *level = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_Contains(root, "Level")
        && LIBMATTI_MC_Nbt_CompoundTag_GetCompound(root, "Level", &level))
        return level;
    return root;
}

LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_FixChunks_SectionsOf(LIBMATTI_MC_Nbt_CompoundTag *root)
{
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf(root);
    if (level == NULL)
        return NULL;
    LIBMATTI_MC_Nbt_ListTag *sections = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "Sections", &sections))
        return sections;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "sections", &sections))
        return sections;
    return NULL;
}