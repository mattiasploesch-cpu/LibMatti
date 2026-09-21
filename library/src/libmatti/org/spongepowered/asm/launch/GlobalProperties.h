// Port of org.spongepowered.asm.launch.GlobalProperties - "Access to underlying global property
// service provided by the current environment" (the mixin blackboard).

#ifndef MATTICRAFT_SP_ASM_LAUNCH_GLOBALPROPERTIES_H
#define MATTICRAFT_SP_ASM_LAUNCH_GLOBALPROPERTIES_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: public static final class Keys
typedef struct LIBMATTI_SP_GlobalProperties_Keys LIBMATTI_SP_GlobalProperties_Keys;

struct LIBMATTI_SP_GlobalProperties_Keys
{
    // Java: private final String name
    const char *name;
    // Java: private IPropertyKey key - resolved once against the service
    int resolved;
    LIBMATTI_SP_IPropertyKey key;
};

extern const LIBMATTI_SP_GlobalProperties_Keys LIBMATTI_SP_GlobalProperties_KEYS[];
#define LIBMATTI_SP_GlobalProperties_KEYS_COUNT 4

// Java: Keys.INIT ("mixin.initialised")
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_INIT(void);
// Java: Keys.AGENTS ("mixin.agents")
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_AGENTS(void);
// Java: Keys.CONFIGS ("mixin.configs")
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_CONFIGS(void);
// Java: Keys.PLATFORM_MANAGER ("mixin.platform")
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_PLATFORM_MANAGER(void);
// Java: public static Keys of(String name) - Keys.of caches by name
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_Of(const char *name);

// Java: public static <T> T get(Keys key) - NULL when unset
void *LIBMATTI_SP_GlobalProperties_Get(LIBMATTI_SP_GlobalProperties_Keys *key);
// Java: public static void put(Keys key, Object value)
void LIBMATTI_SP_GlobalProperties_Put(LIBMATTI_SP_GlobalProperties_Keys *key, void *value);
// Java: public static <T> T get(Keys key, T defaultValue)
void *LIBMATTI_SP_GlobalProperties_GetOrDefault(LIBMATTI_SP_GlobalProperties_Keys *key, void *defaultValue);
// Java: public static String getString(Keys key, String defaultValue)
const char *LIBMATTI_SP_GlobalProperties_GetString(LIBMATTI_SP_GlobalProperties_Keys *key,
                                                   const char *defaultValue);

#endif //MATTICRAFT_SP_ASM_LAUNCH_GLOBALPROPERTIES_H
