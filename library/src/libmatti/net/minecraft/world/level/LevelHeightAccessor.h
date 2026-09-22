// Port of net.minecraft.world.level.LevelHeightAccessor (interface) and
// net.minecraft.core.SectionPos.blockToSectionCoord (the one helper it needs).
//
// Java's interface with defaults becomes a plain struct + free functions the
// Level/LevelChunk share; the two abstract accessors are function pointers so a
// Level or any custom source can override them.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELHEIGHTACCESSOR_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELHEIGHTACCESSOR_H

#include "libmatti/net/minecraft/core/SectionPos.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: SectionPos.blockToSectionCoord(int) = coord >> 4 and
// sectionToBlockCoord(int) = coord << 4 - the macros live on SectionPos.h and
// come with that include (Level.h pulls it through the chunk headers).

// Java: public interface LevelHeightAccessor - the port keeps the two abstract
// accessors in the struct and implements every default as a free function
typedef struct LIBMATTI_MC_LevelHeightAccessor
{
    // Java: int getHeight() / int getMinY() - the abstract pair
    int height;
    int minY;
} LIBMATTI_MC_LevelHeightAccessor;

// Java: public int getMaxY() = getMinY() + getHeight() - 1
int LIBMATTI_MC_LevelHeightAccessor_GetMaxY(const LIBMATTI_MC_LevelHeightAccessor *accessor);
// Java: public int getSectionsCount() = getMaxSectionY() - getMinSectionY() + 1
int LIBMATTI_MC_LevelHeightAccessor_GetSectionsCount(const LIBMATTI_MC_LevelHeightAccessor *accessor);
// Java: public int getMinSectionY() = SectionPos.blockToSectionCoord(getMinY())
int LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor);
// Java: public int getMaxSectionY() = SectionPos.blockToSectionCoord(getMaxY())
int LIBMATTI_MC_LevelHeightAccessor_GetMaxSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor);
// Java: public boolean isInsideBuildHeight(int)
bool LIBMATTI_MC_LevelHeightAccessor_IsInsideBuildHeight(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y);
// Java: public boolean isOutsideBuildHeight(int)
bool LIBMATTI_MC_LevelHeightAccessor_IsOutsideBuildHeight(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y);
// Java: public int getSectionIndex(int) = getSectionIndexFromSectionY(blockToSectionCoord(y))
int LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(const LIBMATTI_MC_LevelHeightAccessor *accessor, int y);
// Java: public int getSectionIndexFromSectionY(int) = sectionY - getMinSectionY()
int LIBMATTI_MC_LevelHeightAccessor_GetSectionIndexFromSectionY(const LIBMATTI_MC_LevelHeightAccessor *accessor, int sectionY);
// Java: public int getSectionYFromSectionIndex(int) = index + getMinSectionY()
int LIBMATTI_MC_LevelHeightAccessor_GetSectionYFromSectionIndex(const LIBMATTI_MC_LevelHeightAccessor *accessor, int index);
// Java: public static LevelHeightAccessor create(int minY, int height)
LIBMATTI_MC_LevelHeightAccessor LIBMATTI_MC_LevelHeightAccessor_Create(int minY, int height);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELHEIGHTACCESSOR_H
