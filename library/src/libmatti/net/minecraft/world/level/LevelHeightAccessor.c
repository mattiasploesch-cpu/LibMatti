// Port of net.minecraft.world.level.LevelHeightAccessor.

#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"

#include "libmatti/net/minecraft/core/SectionPos.h"

int LIBMATTI_MC_LevelHeightAccessor_GetMaxY(const LIBMATTI_MC_LevelHeightAccessor *accessor)
{
    return accessor->minY + accessor->height - 1;
}

int LIBMATTI_MC_LevelHeightAccessor_GetSectionsCount(const LIBMATTI_MC_LevelHeightAccessor *accessor)
{
    return LIBMATTI_MC_LevelHeightAccessor_GetMaxSectionY(accessor)
           - LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(accessor) + 1;
}

int LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor)
{
    return LIBMATTI_MC_SectionPos_BlockToSectionCoord(accessor->minY);
}

int LIBMATTI_MC_LevelHeightAccessor_GetMaxSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor)
{
    return LIBMATTI_MC_SectionPos_BlockToSectionCoord(LIBMATTI_MC_LevelHeightAccessor_GetMaxY(accessor));
}

bool LIBMATTI_MC_LevelHeightAccessor_IsInsideBuildHeight(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y)
{
    return y >= accessor->minY && y <= LIBMATTI_MC_LevelHeightAccessor_GetMaxY(accessor);
}

bool LIBMATTI_MC_LevelHeightAccessor_IsOutsideBuildHeight(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y)
{
    return y < accessor->minY || y > LIBMATTI_MC_LevelHeightAccessor_GetMaxY(accessor);
}

int LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y)
{
    return LIBMATTI_MC_LevelHeightAccessor_GetSectionIndexFromSectionY(
        accessor, LIBMATTI_MC_SectionPos_BlockToSectionCoord(y));
}

int LIBMATTI_MC_LevelHeightAccessor_GetSectionIndexFromSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor, int sectionY)
{
    return sectionY - LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(accessor);
}

int LIBMATTI_MC_LevelHeightAccessor_GetSectionYFromSectionIndex(const LIBMATTI_MC_LevelHeightAccessor *accessor, int index)
{
    return index + LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(accessor);
}

LIBMATTI_MC_LevelHeightAccessor LIBMATTI_MC_LevelHeightAccessor_Create(int minY, int height)
{
    LIBMATTI_MC_LevelHeightAccessor accessor = {height, minY};
    return accessor;
}
