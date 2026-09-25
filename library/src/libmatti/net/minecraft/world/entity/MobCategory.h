// Port of net.minecraft.world.entity.MobCategory (the spawn classification).

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
    LIBMATTI_MC_MobCategory_MONSTER,              // "monster", 70, not friendly, not persistent
    LIBMATTI_MC_MobCategory_CREATURE,             // "creature", 10, friendly, persistent
    LIBMATTI_MC_MobCategory_AMBIENT,              // "ambient", 15, friendly, not persistent
    LIBMATTI_MC_MobCategory_AXOLOTLS,             // "axolotls", 5, friendly, persistent
    LIBMATTI_MC_MobCategory_UNDERGROUND_WATER_CREATURE, // "underground_water_creature", 5
    LIBMATTI_MC_MobCategory_WATER_CREATURE,       // "water_creature", 5
    LIBMATTI_MC_MobCategory_WATER_AMBIENT,        // "water_ambient", 20
    LIBMATTI_MC_MobCategory_MISC,                 // "misc", -1, friendly, persistent
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

#endif //MATTICRAFT_MC_WORLD_ENTITY_MOBCATEGORY_H
