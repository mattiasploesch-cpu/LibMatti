


#ifndef MATTICRAFT_MC_WORLD_ENTITY_MOBCATEGORY_H
#define MATTICRAFT_MC_WORLD_ENTITY_MOBCATEGORY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum MobCategory implements StringRepresentable - the port keeps
// the name, the cap (max * perPlayer), the friendly flag and the persistent flag
typedef enum LIBMATTI_MC_MobCategory
{
    LIBMATTI_MC_MobCategory_MONSTER,

    LIBMATTI_MC_MobCategory_CREATURE,

    LIBMATTI_MC_MobCategory_AMBIENT,

    LIBMATTI_MC_MobCategory_AXOLOTLS,

    LIBMATTI_MC_MobCategory_UNDERGROUND_WATER_CREATURE,

    LIBMATTI_MC_MobCategory_WATER_CREATURE,

    LIBMATTI_MC_MobCategory_WATER_AMBIENT,

    LIBMATTI_MC_MobCategory_MISC,

    LIBMATTI_MC_MobCategory_COUNT,
} LIBMATTI_MC_MobCategory;

// Java: the name (the serialisation key)
const char *LIBMATTI_MC_MobCategory_GetName(LIBMATTI_MC_MobCategory category);
// Java: the spawn cap (Misc == -1 means unlimited / not tracked)
int LIBMATTI_MC_MobCategory_GetMax(LIBMATTI_MC_MobCategory category);
// Java: isFriendly
bool LIBMATTI_MC_MobCategory_IsFriendly(LIBMATTI_MC_MobCategory category);
// Java: isPersistent
bool LIBMATTI_MC_MobCategory_IsPersistent(LIBMATTI_MC_MobCategory category);
// Java: getMaxInstancesPerChunk - the no-count category carries 0
int LIBMATTI_MC_MobCategory_GetMaxInstancesPerChunk(LIBMATTI_MC_MobCategory category);
// Java: byName - MISC when the name is unknown (Java's enum lookup)
LIBMATTI_MC_MobCategory LIBMATTI_MC_MobCategory_ByName(const char *name);

#ifdef __cplusplus
}
#endif

#endif
