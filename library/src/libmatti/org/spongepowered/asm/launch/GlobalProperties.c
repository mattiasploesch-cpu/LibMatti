// Port of org.spongepowered.asm.launch.GlobalProperties.

#include "libmatti/org/spongepowered/asm/launch/GlobalProperties.h"

#include <stdlib.h>
#include <string.h>

// Java: the Keys constants (Keys.of caches by name; the port keeps the struct array growable)
static LIBMATTI_SP_GlobalProperties_Keys *KEYS = NULL;
static size_t keyCount = 0;

static void keys_init(void)
{
    if (KEYS != NULL) return;

    static const char *const NAMES[] = {"mixin.initialised", "mixin.agents", "mixin.configs", "mixin.platform"};
    KEYS = malloc(sizeof(LIBMATTI_SP_GlobalProperties_Keys) * 4);
    for (size_t i = 0; i < 4; i++)
    {
        KEYS[i].name = NAMES[i];
        KEYS[i].resolved = 0;
        memset(&KEYS[i].key, 0, sizeof(LIBMATTI_SP_IPropertyKey));
    }
    keyCount = 4;
}

const LIBMATTI_SP_GlobalProperties_Keys LIBMATTI_SP_GlobalProperties_KEYS[] = {
    {"mixin.initialised", 0, {NULL}},
    {"mixin.agents", 0, {NULL}},
    {"mixin.configs", 0, {NULL}},
    {"mixin.platform", 0, {NULL}},
};

LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_INIT(void)
{
    keys_init();
    return &KEYS[0];
}

LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_AGENTS(void)
{
    keys_init();
    return &KEYS[1];
}

LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_CONFIGS(void)
{
    keys_init();
    return &KEYS[2];
}

LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_PLATFORM_MANAGER(void)
{
    keys_init();
    return &KEYS[3];
}

// Java: public static Keys of(String name) { return new Keys(name); } - Keys.of caches by name
LIBMATTI_SP_GlobalProperties_Keys *LIBMATTI_SP_GlobalProperties_Keys_Of(const char *name)
{
    for (size_t i = 0; i < sizeof(KEYS) / sizeof(*KEYS); i++)
        if (strcmp(KEYS[i].name, name) == 0) return &KEYS[i];

    KEYS = realloc(KEYS, sizeof(LIBMATTI_SP_GlobalProperties_Keys) * (keyCount + 1));
    KEYS[keyCount].name = strdup(name);
    KEYS[keyCount].resolved = 0;
    memset(&KEYS[keyCount].key, 0, sizeof(LIBMATTI_SP_IPropertyKey));
    return &KEYS[keyCount++];
}

// Java: private static IGlobalPropertyService getService()
static LIBMATTI_SP_IGlobalPropertyService *property_service(void)
{
    return LIBMATTI_SP_MixinService_GetGlobalPropertyService();
}

// Java: key.resolve(service)
static LIBMATTI_SP_IPropertyKey resolve(LIBMATTI_SP_GlobalProperties_Keys *key)
{
    if (!key->resolved)
    {
        LIBMATTI_SP_IGlobalPropertyService *service = property_service();
        if (service != NULL)
        {
            key->key = service->resolveKey(service->self, key->name);
            key->resolved = 1;
        }
    }
    return key->key;
}

// Java: public static <T> T get(Keys key)
void *LIBMATTI_SP_GlobalProperties_Get(LIBMATTI_SP_GlobalProperties_Keys *key)
{
    LIBMATTI_SP_IGlobalPropertyService *service = property_service();
    if (service == NULL) return NULL;
    return service->getProperty(service->self, resolve(key));
}

// Java: public static void put(Keys key, Object value)
void LIBMATTI_SP_GlobalProperties_Put(LIBMATTI_SP_GlobalProperties_Keys *key, void *value)
{
    LIBMATTI_SP_IGlobalPropertyService *service = property_service();
    if (service == NULL) return;
    service->setProperty(service->self, resolve(key), value);
}

// Java: public static <T> T get(Keys key, T defaultValue)
void *LIBMATTI_SP_GlobalProperties_GetOrDefault(LIBMATTI_SP_GlobalProperties_Keys *key, void *defaultValue)
{
    LIBMATTI_SP_IGlobalPropertyService *service = property_service();
    if (service == NULL) return defaultValue;
    void *value = service->getProperty(service->self, resolve(key));
    return value != NULL ? value : defaultValue;
}

// Java: public static String getString(Keys key, String defaultValue)
const char *LIBMATTI_SP_GlobalProperties_GetString(LIBMATTI_SP_GlobalProperties_Keys *key,
                                                   const char *defaultValue)
{
    LIBMATTI_SP_IGlobalPropertyService *service = property_service();
    if (service == NULL) return defaultValue;
    return service->getPropertyString(service->self, resolve(key), defaultValue);
}
