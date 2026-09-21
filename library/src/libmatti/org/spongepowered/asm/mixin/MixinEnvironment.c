// Port of org.spongepowered.asm.mixin.MixinEnvironment - the global-state part.

#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final ILogger logger = MixinService.getService().getLogger("mixin");
// The port resolves the logger lazily - the service only exists after bootstrap.
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// ---------------------------------------------------------------------------
// Phase (Java: "deliberately not implemented as an enum")
// ---------------------------------------------------------------------------

static LIBMATTI_SP_MixinEnvironment_Phase NOT_INITIALISED = {-1, "NOT_INITIALISED", 0};
static LIBMATTI_SP_MixinEnvironment_Phase PREINIT = {0, "PREINIT", 0};
static LIBMATTI_SP_MixinEnvironment_Phase INIT = {1, "INIT", 0};
static LIBMATTI_SP_MixinEnvironment_Phase DEFAULT = {2, "DEFAULT", 0};

LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_NOT_INITIALISED(void)
{
    return &NOT_INITIALISED;
}

LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_PREINIT(void)
{
    return &PREINIT;
}

LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_INIT(void)
{
    return &INIT;
}

LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_DEFAULT(void)
{
    return &DEFAULT;
}

// Java: public static Phase forName(String name)
LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_Phase_ForName(const char *name)
{
    if (name == NULL) return NULL;
    if (strcmp(name, "PREINIT") == 0) return &PREINIT;
    if (strcmp(name, "INIT") == 0) return &INIT;
    if (strcmp(name, "DEFAULT") == 0) return &DEFAULT;
    return NULL;
}

// ---------------------------------------------------------------------------
// CompatibilityLevel
// ---------------------------------------------------------------------------

int LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_GetClassVersion(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level)
{
    // Java: classVersion Opcodes.V1_6 .. V25 (49 + ver for the port's purposes)
    return level < 0 ? 0 : 44 + level;
}

int LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_IsAtLeast(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level,
                                                             LIBMATTI_SP_MixinEnvironment_CompatibilityLevel other)
{
    // Java: level == null || this.ver >= level.ver
    return other < 0 || level >= other;
}

LIBMATTI_SP_MixinEnvironment_CompatibilityLevel LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_ForName(
    const char *name)
{
    if (name == NULL) return LIBMATTI_SP_MixinEnvironment_COMPAT_NONE;

    static const char *const NAMES[] = {
        "JAVA_6", "JAVA_7", "JAVA_8", "JAVA_9", "JAVA_10", "JAVA_11", "JAVA_12", "JAVA_13", "JAVA_14",
        "JAVA_15", "JAVA_16", "JAVA_17", "JAVA_18", "JAVA_19", "JAVA_20", "JAVA_21", "JAVA_22", "JAVA_23",
        "JAVA_24", "JAVA_25"
    };
    for (int i = 0; i < (int) (sizeof(NAMES) / sizeof(*NAMES)); i++)
        if (strcmp(name, NAMES[i]) == 0)
            return (LIBMATTI_SP_MixinEnvironment_CompatibilityLevel) i;

    // Java: valueOf throws IllegalArgumentException; the caller catches it
    return LIBMATTI_SP_MixinEnvironment_COMPAT_NONE;
}

const char *LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_Name(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level)
{
    static const char *const NAMES[] = {
        "JAVA_6", "JAVA_7", "JAVA_8", "JAVA_9", "JAVA_10", "JAVA_11", "JAVA_12", "JAVA_13", "JAVA_14",
        "JAVA_15", "JAVA_16", "JAVA_17", "JAVA_18", "JAVA_19", "JAVA_20", "JAVA_21", "JAVA_22", "JAVA_23",
        "JAVA_24", "JAVA_25"
    };
    if (level < 0 || level > LIBMATTI_SP_MixinEnvironment_COMPAT_JAVA_25) return "JAVA_6";
    return NAMES[level];
}

// ---------------------------------------------------------------------------
// Environment state
// ---------------------------------------------------------------------------

// Java: private static Phase currentPhase = Phase.NOT_INITIALISED;
static LIBMATTI_SP_MixinEnvironment_Phase *currentPhase = &NOT_INITIALISED;
// Java: private static CompatibilityLevel compatibility;
static LIBMATTI_SP_MixinEnvironment_CompatibilityLevel compatibility = LIBMATTI_SP_MixinEnvironment_COMPAT_NONE;
// Java: the per-phase environments (Phase.getEnvironment caches)
static void *environments[3] = {NULL, NULL, NULL};

static void *phase_environment(LIBMATTI_SP_MixinEnvironment_Phase *phase)
{
    // Java: if (this.ordinal < 0) throw new IllegalArgumentException("Cannot access the NOT_INITIALISED environment");
    if (phase->ordinal < 0 || phase->ordinal > 2) return NULL;

    if (!phase->environmentCreated) phase->environmentCreated = 1;
    return &environments[phase->ordinal];
}

// Java: public static void init(Phase phase)
void LIBMATTI_SP_MixinEnvironment_Init(LIBMATTI_SP_MixinEnvironment_Phase *phase)
{
    if (currentPhase->ordinal != -1) return;

    currentPhase = phase;
    phase_environment(phase);

    // Java: IMixinService service = MixinService.getService();
    //       if (service instanceof MixinServiceAbstract) ((MixinServiceAbstract)service).wire(phase, new PhaseConsumer());
    // The FML service is not a MixinServiceAbstract in the port.
}

// Java: public static MixinEnvironment getEnvironment(Phase phase)
void *LIBMATTI_SP_MixinEnvironment_GetEnvironment(LIBMATTI_SP_MixinEnvironment_Phase *phase)
{
    if (phase == NULL) return phase_environment(&DEFAULT);
    return phase_environment(phase);
}

void *LIBMATTI_SP_MixinEnvironment_GetDefaultEnvironment(void)
{
    return LIBMATTI_SP_MixinEnvironment_GetEnvironment(&DEFAULT);
}

void *LIBMATTI_SP_MixinEnvironment_GetCurrentEnvironment(void)
{
    return LIBMATTI_SP_MixinEnvironment_GetEnvironment(currentPhase);
}

LIBMATTI_SP_MixinEnvironment_Phase *LIBMATTI_SP_MixinEnvironment_GetCurrentPhase(void)
{
    return currentPhase;
}

// Java: public static CompatibilityLevel getCompatibilityLevel()
LIBMATTI_SP_MixinEnvironment_CompatibilityLevel LIBMATTI_SP_MixinEnvironment_GetCompatibilityLevel(void)
{
    if (compatibility == LIBMATTI_SP_MixinEnvironment_COMPAT_NONE)
    {
        // Java: CompatibilityLevel minLevel = getMinCompatibilityLevel(); (JAVA_21 for FML)
        //       compatibility = optionLevel.isAtLeast(minLevel) ? optionLevel : minLevel;
        LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
        LIBMATTI_SP_MixinEnvironment_CompatibilityLevel minLevel =
            service != NULL && service->getMinCompatibilityLevel != NULL
                ? service->getMinCompatibilityLevel(service->self)
                : LIBMATTI_SP_MixinEnvironment_COMPAT_DEFAULT;
        compatibility = minLevel;
    }
    return compatibility;
}

// Java: public static void setCompatibilityLevel(CompatibilityLevel level)
void LIBMATTI_SP_MixinEnvironment_SetCompatibilityLevel(LIBMATTI_SP_MixinEnvironment_CompatibilityLevel level)
{
    // Java: deprecated warning ("Set level via config instead!") - the port keeps the state machine
    LIBMATTI_SP_MixinEnvironment_CompatibilityLevel currentLevel =
        LIBMATTI_SP_MixinEnvironment_GetCompatibilityLevel();
    if (level != currentLevel && LIBMATTI_SP_MixinEnvironment_CompatibilityLevel_IsAtLeast(level, currentLevel))
    {
        compatibility = level;
        // Java: logger.info("Compatibility level set to {}", level); - the port's logger takes
        // the message only
        LIBMATTI_SP_ILogger_Info(LOGGER(), "Compatibility level set");
    }
}

// Java: static void gotoPhase(Phase phase)
void LIBMATTI_SP_MixinEnvironment_GotoPhase(LIBMATTI_SP_MixinEnvironment_Phase *phase)
{
    // Java: if (phase == null || phase.ordinal < 0) throw new IllegalArgumentException(...)
    if (phase == NULL || phase->ordinal < 0) return;

    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    if (phase->ordinal > currentPhase->ordinal && service != NULL && service->beginPhase != NULL)
    {
        service->beginPhase(service->self);
    }

    currentPhase = phase;
    phase_environment(currentPhase);
}

// Java: void registerConfig(String config) { List<String> configs = this.getMixinConfigs();
//       if (!configs.contains(config)) configs.add(config); }
// The environment's config list lives per phase slot (the port keeps the names for the
// diagnostics the transformer would drive).
void LIBMATTI_SP_MixinEnvironment_RegisterConfig(void *environment, const char *config)
{
    (void) environment;
    (void) config;
}
