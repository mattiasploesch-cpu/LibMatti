// Port of net.minecraft.world.entity.MobCategory (implementation).

#include "libmatti/net/minecraft/world/entity/MobCategory.h"

#include <string.h>

static const struct
{
    const char *name;
    int max;
    bool isFriendly;
    bool isPersistent; // Java: noPersistent flag inverted
    int noDespawnValue;
} CATEGORY_DATA[LIBMATTI_MC_MobCategory_COUNT] = {
    // Java: MONSTER("monster", 70, false, false, 128)
    [LIBMATTI_MC_MobCategory_MONSTER] = {"monster", 70, false, false, 128},
    // Java: CREATURE("creature", 10, true, true, 128)
    [LIBMATTI_MC_MobCategory_CREATURE] = {"creature", 10, true, true, 128},
    // Java: AMBIENT("ambient", 15, true, false, 128)
    [LIBMATTI_MC_MobCategory_AMBIENT] = {"ambient", 15, true, false, 128},
    // Java: AXOLOTLS("axolotls", 5, true, true, 128)
    [LIBMATTI_MC_MobCategory_AXOLOTLS] = {"axolotls", 5, true, true, 128},
    // Java: UNDERGROUND_WATER_CREATURE("underground_water_creature", 5, true, true, 128)
    [LIBMATTI_MC_MobCategory_UNDERGROUND_WATER_CREATURE] = {"underground_water_creature", 5, true, true, 128},
    // Java: WATER_CREATURE("water_creature", 5, true, true, 128)
    [LIBMATTI_MC_MobCategory_WATER_CREATURE] = {"water_creature", 5, true, true, 128},
    // Java: WATER_AMBIENT("water_ambient", 20, true, false, 64)
    [LIBMATTI_MC_MobCategory_WATER_AMBIENT] = {"water_ambient", 20, true, false, 64},
    // Java: MISC("misc", -1, true, true, 128)
    [LIBMATTI_MC_MobCategory_MISC] = {"misc", -1, true, true, 128},
};

const char *LIBMATTI_MC_MobCategory_GetName(LIBMATTI_MC_MobCategory category)
{
    if (category < 0 || category >= LIBMATTI_MC_MobCategory_COUNT)
        return "misc";
    return CATEGORY_DATA[category].name;
}

int LIBMATTI_MC_MobCategory_GetMax(LIBMATTI_MC_MobCategory category)
{
    if (category < 0 || category >= LIBMATTI_MC_MobCategory_COUNT)
        return -1;
    return CATEGORY_DATA[category].max;
}

bool LIBMATTI_MC_MobCategory_IsFriendly(LIBMATTI_MC_MobCategory category)
{
    if (category < 0 || category >= LIBMATTI_MC_MobCategory_COUNT)
        return true;
    return CATEGORY_DATA[category].isFriendly;
}

bool LIBMATTI_MC_MobCategory_IsPersistent(LIBMATTI_MC_MobCategory category)
{
    if (category < 0 || category >= LIBMATTI_MC_MobCategory_COUNT)
        return true;
    return CATEGORY_DATA[category].isPersistent;
}

int LIBMATTI_MC_MobCategory_GetMaxInstancesPerChunk(LIBMATTI_MC_MobCategory category)
{
    if (category < 0 || category >= LIBMATTI_MC_MobCategory_COUNT)
        return 0;
    // Java: return this.max == -1 ? 0 : this.max;
    return CATEGORY_DATA[category].max == -1 ? 0 : CATEGORY_DATA[category].max;
}

LIBMATTI_MC_MobCategory LIBMATTI_MC_MobCategory_ByName(const char *name)
{
    if (name != NULL)
    {
        for (int i = 0; i < LIBMATTI_MC_MobCategory_COUNT; i++)
        {
            if (strcmp(CATEGORY_DATA[i].name, name) == 0)
                return (LIBMATTI_MC_MobCategory) i;
        }
    }
    return LIBMATTI_MC_MobCategory_MISC;
}
