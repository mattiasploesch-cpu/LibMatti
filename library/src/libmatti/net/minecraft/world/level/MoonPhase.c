// Port of net.minecraft.world.level.MoonPhase (implementation).

#include "libmatti/net/minecraft/world/level/MoonPhase.h"

static const char *NAMES[LIBMATTI_MC_MoonPhase_COUNT] = {
    "full_moon",     "waning_gibbous", "third_quarter",  "waning_crescent",
    "new_moon",      "waxing_crescent", "first_quarter", "waxing_gibbous"
};

const char *LIBMATTI_MC_MoonPhase_GetSerializedName(LIBMATTI_MC_MoonPhase phase)
{
    if (phase < 0 || phase >= LIBMATTI_MC_MoonPhase_COUNT)
        return NAMES[0];
    return NAMES[phase];
}

LIBMATTI_MC_MoonPhase LIBMATTI_MC_MoonPhase_ByIndex(int index)
{
    int wrapped = index % LIBMATTI_MC_MoonPhase_COUNT;
    if (wrapped < 0)
        wrapped += LIBMATTI_MC_MoonPhase_COUNT;
    return (LIBMATTI_MC_MoonPhase) wrapped;
}
