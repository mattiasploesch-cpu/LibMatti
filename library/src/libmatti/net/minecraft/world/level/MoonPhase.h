// Port of net.minecraft.world.level.MoonPhase.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_MOONPHASE_H
#define MATTICRAFT_MC_WORLD_LEVEL_MOONPHASE_H

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public enum MoonPhase implements StringRepresentable
typedef enum LIBMATTI_MC_MoonPhase
{
    LIBMATTI_MC_MoonPhase_FULL_MOON = 0,
    LIBMATTI_MC_MoonPhase_WANING_GIBBOUS = 1,
    LIBMATTI_MC_MoonPhase_THIRD_QUARTER = 2,
    LIBMATTI_MC_MoonPhase_WANING_CRESCENT = 3,
    LIBMATTI_MC_MoonPhase_NEW_MOON = 4,
    LIBMATTI_MC_MoonPhase_WAXING_CRESCENT = 5,
    LIBMATTI_MC_MoonPhase_FIRST_QUARTER = 6,
    LIBMATTI_MC_MoonPhase_WAXING_GIBBOUS = 7
} LIBMATTI_MC_MoonPhase;

// Java: public static final int COUNT = values().length;
#define LIBMATTI_MC_MoonPhase_COUNT 8
// Java: public static final int PHASE_LENGTH = 24000;
#define LIBMATTI_MC_MoonPhase_PHASE_LENGTH 24000

// Java: public int index()
static inline int LIBMATTI_MC_MoonPhase_Index(LIBMATTI_MC_MoonPhase phase)
{
    return (int) phase;
}

// Java: public String getSerializedName()
const char *LIBMATTI_MC_MoonPhase_GetSerializedName(LIBMATTI_MC_MoonPhase phase);

// Java: public static MoonPhase byIndex(int index) - floorMod over the phase count.
LIBMATTI_MC_MoonPhase LIBMATTI_MC_MoonPhase_ByIndex(int index);

#ifdef __cplusplus
}
#endif

#endif
