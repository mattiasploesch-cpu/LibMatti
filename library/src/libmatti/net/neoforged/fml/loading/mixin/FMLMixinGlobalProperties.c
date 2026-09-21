// Port of net.neoforged.fml.loading.mixin.FMLMixinGlobalProperties.
// Java: private static final Map<String, Object> PROPERTIES = new HashMap<>() guarded by
// synchronized(PROPERTIES); the port keeps one mutex around the same map.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinGlobalProperties.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *name;
    void *value;
} PropertyEntry;

static PropertyEntry *properties = NULL;
static size_t propertyCount = 0;
static pthread_mutex_t PROPERTIES_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static PropertyEntry *find_entry(const char *name)
{
    for (size_t i = 0; i < propertyCount; i++)
        if (strcmp(properties[i].name, name) == 0) return &properties[i];
    return NULL;
}

// Java: IPropertyKey resolveKey(String name) { return new StringKey(name); }
static LIBMATTI_SP_IPropertyKey resolve_key(void *self, const char *name)
{
    (void) self;
    LIBMATTI_SP_IPropertyKey key = {name};
    return key;
}

// Java: private String getKeyName(IPropertyKey key) { return ((StringKey) key).name(); }
static void *get_property(void *self, LIBMATTI_SP_IPropertyKey key)
{
    (void) self;

    pthread_mutex_lock(&PROPERTIES_MUTEX);
    PropertyEntry *entry = find_entry(key.name);
    void *value = entry != NULL ? entry->value : NULL;
    pthread_mutex_unlock(&PROPERTIES_MUTEX);
    return value;
}

static void set_property(void *self, LIBMATTI_SP_IPropertyKey key, void *value)
{
    (void) self;

    pthread_mutex_lock(&PROPERTIES_MUTEX);
    PropertyEntry *entry = find_entry(key.name);
    if (entry != NULL)
    {
        entry->value = value;
    }
    else
    {
        properties = realloc(properties, sizeof(PropertyEntry) * (propertyCount + 1));
        properties[propertyCount].name = strdup(key.name);
        properties[propertyCount].value = value;
        propertyCount++;
    }
    pthread_mutex_unlock(&PROPERTIES_MUTEX);
}

static const char *get_property_string(void *self, LIBMATTI_SP_IPropertyKey key, const char *defaultValue)
{
    (void) self;

    pthread_mutex_lock(&PROPERTIES_MUTEX);
    PropertyEntry *entry = find_entry(key.name);
    const char *value = entry != NULL ? entry->value : NULL;
    pthread_mutex_unlock(&PROPERTIES_MUTEX);

    // Java: Objects.requireNonNullElse((String) PROPERTIES.get(getKeyName(key)), defaultValue)
    return value != NULL ? value : defaultValue;
}

// Java: public class FMLMixinGlobalProperties implements IGlobalPropertyService
LIBMATTI_SP_IGlobalPropertyService *LIBMATTI_FML_FMLMixinGlobalProperties_Instance(void)
{
    static LIBMATTI_SP_IGlobalPropertyService service;
    static int initialised = 0;

    if (!initialised)
    {
        service.self = NULL;
        service.resolveKey = resolve_key;
        service.getProperty = get_property;
        service.setProperty = set_property;
        service.getPropertyString = get_property_string;
        initialised = 1;
    }
    return &service;
}
