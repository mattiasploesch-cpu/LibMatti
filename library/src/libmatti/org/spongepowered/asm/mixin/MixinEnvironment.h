// Port of org.spongepowered.asm.mixin.MixinEnvironment (the global-state part of
// sponge-mixin 0.17.3+mixin.0.8.7). The transformer-facing half of the class (options,
// token providers, remappers) belongs to the bytecode transformer the port replaces with
// the native hook table; Phase, Side, CompatibilityLevel, the phase transitions
// (init/gotoPhase) and the compatibility level are exactly what the loader drives.

#ifndef MATTICRAFT_SP_ASM_MIXIN_MIXINENVIRONMENT_H
#define MATTICRAFT_SP_ASM_MIXIN_MIXINENVIRONMENT_H

// Java: public static final class Phase - "deliberately not implemented as an enum"
typedef struct LIBMATTI_SP_MixinEnvironment_Phase LIBMATTI_SP_MixinEnvironment_Phase;

struct LIBMATTI_SP_MixinEnvironment_Phase
{
    // Java: final int ordinal
    int ordinal;
    // Java: final String name
    const char *name;
    // Java: the environment per phase; the port keeps the environments by ordinal
    int environmentCreated;
};

// Java: Phase.NOT_INITIALISED (ordinal -1)
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_NOT_INITIALISED(void);
// Java: Phase.PREINIT
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_PREINIT(void);
// Java: Phase.INIT
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_INIT(void);
// Java: Phase.DEFAULT
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_DEFAULT(void);
// Java: public static Phase forName(String name)
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_ForName(const char *name);

// Java: public static enum Side
typedef enum
{
    LIBMATTI_SP_MixinEnvironment_SIDE_UNKNOWN,
    LIBMATTI_SP_MixinEnvironment_SIDE_CLIENT,
    LIBMATTI_SP_MixinEnvironment_SIDE_SERVER
} LIBMATTI_SP_MixinEnvironment_Side;

// Java: public static enum CompatibilityLevel (the levels the FML layer touches)
typedef enum
{
    LIBMATTI_SP_MixinEnvironment_COMPAT_NONE = -1, // Java: a null max level (service returns null)
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_6,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_7,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_8,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_9,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_10,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_11,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_12,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_13,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_14,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_15,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_16,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_17,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_18,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_19,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_20,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_21,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_22,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_23,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_24,
    LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_25
} LIBMATTI_SP_MixinEnvironment_CompatibilityLevel;

// Java: CompatibilityLevel.DEFAULT = JAVA_6
#define LIBMATTI_SP_MixinEnvironment_COMPAT_DEFAULT LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_6
// Java: CompatibilityLevel.MAX_SUPPORTED = JAVA_13
#define LIBMATTI_SP_MixinEnvironment_COMPAT_MAX_SUPPORTED LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_13

// Java: public static enum CompatibilityLevel helpers
int LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_GetClassVersion(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level);
int LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_IsAtLeast(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level,
                                                             LIBMATTI_SP_MixinEnvironment_CompatibilityLevel other);
// Java: public static CompatibilityLevel forName(String) - via valueOf; NONE when invalid
LIBMATTI_SP_MixinEnvironment_CompatibilityLevel LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_ForName(
    const char *name);
const char *LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_Name(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level);

// Java: public static void init(Phase phase)
void LIBMATTI_SP_MixinEnvironment_Init(LIBMATTI_SP_MixinEnvironment_Phase *phase);
// Java: public static MixinEnvironment getEnvironment(Phase phase)
void *LIBMATTI_SP_MixinEnvironment_GetEnvironment(LIBMATTI_SP_MixinEnvironment_Phase *phase);
// Java: public static MixinEnvironment getDefaultEnvironment()
void *LIBMATTI_SP_MixinEnvironment_GetDefaultEnvironment(void);
// Java: public static MixinEnvironment getCurrentEnvironment()
void *LIBMATTI_SP_MixinEnvironment_GetCurrentEnvironment(void);
// Java: the phase the environment is in
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_GetCurrentPhase(void);
// Java: public static CompatibilityLevel getCompatibilityLevel()
LIBMATTI_SP_MixinEnvironment_CompatibilityLevel LIBMATTI_SP_MixinEnvironment_GetCompatibilityLevel(void);
// Java: public static void setCompatibilityLevel(CompatibilityLevel level)
void LIBMATTI_SP_MixinEnvironment_SetCompatibilityLevel(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level);
// Java: static void gotoPhase(Phase phase)
void LIBMATTI_SP_MixinEnvironment_GotoPhase(LIBMATTI_SP_MixinEnvironment_Phase *phase);
// Java: void registerConfig(String config) (package-private; Mixins calls it)
void LIBMATTI_SP_MixinEnvironment_RegisterConfig(void *environment, const char *config);

#endif //MATTICRAFT_SP_ASM_MIXIN_MIXINENVIRONMENT_H
