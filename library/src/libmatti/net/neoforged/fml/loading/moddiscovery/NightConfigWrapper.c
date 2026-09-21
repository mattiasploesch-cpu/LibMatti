#include "libmatti/net/neoforged/fml/loading/moddiscovery/NightConfigWrapper.h"

#include <stdint.h>

#include "libmatti/net/neoforged/neoforgespi/locating/InvalidModFileException.h"

#include <stdlib.h>
#include <string.h>

struct LIBMATTI_FML_NightConfigWrapper
{
    // Java: private final UnmodifiableConfig config
    LIBMATTI_NC_Config *config;
    // Java: private IModFileInfo file
    LIBMATTI_NEOFORGESPI_IModFileInfo *file;
    LIBMATTI_NEOFORGESPI_IConfigurable configurable;
};

// Java: public NightConfigWrapper(UnmodifiableConfig config)
LIBMATTI_FML_NightConfigWrapper *LIBMATTI_FML_NightConfigWrapper_New(LIBMATTI_NC_Config *config)
{
    // Java: if (config instanceof ConcurrentConfig) throw new IllegalArgumentException(...)
    LIBMATTI_FML_NightConfigWrapper *wrapper = calloc(1, sizeof(LIBMATTI_FML_NightConfigWrapper));
    wrapper->config = config;
    wrapper->configurable.self = wrapper;
    wrapper->configurable.getConfigElement = LIBMATTI_FML_NightConfigWrapper_GetConfigElement;
    wrapper->configurable.getConfigList = LIBMATTI_FML_NightConfigWrapper_GetConfigList;
    return wrapper;
}

// Java: public NightConfigWrapper setFile(IModFileInfo file)
LIBMATTI_FML_NightConfigWrapper *LIBMATTI_FML_NightConfigWrapper_SetFile(
    LIBMATTI_FML_NightConfigWrapper *wrapper, LIBMATTI_NEOFORGESPI_IModFileInfo *file)
{
    wrapper->file = file;
    return wrapper;
}

// Java: the wrapper used as an IConfigurable
LIBMATTI_NEOFORGESPI_IConfigurable *LIBMATTI_FML_NightConfigWrapper_AsConfigurable(
    LIBMATTI_FML_NightConfigWrapper *wrapper)
{
    return wrapper != NULL ? &wrapper->configurable : NULL;
}

// Java: the method reference configWrapper::setFile
void LIBMATTI_FML_NightConfigWrapper_SetFileConsumer(LIBMATTI_NEOFORGESPI_IModFileInfo *file, void *userdata)
{
    LIBMATTI_FML_NightConfigWrapper_SetFile(userdata, file);
}

void LIBMATTI_FML_NightConfigWrapper_Free(LIBMATTI_FML_NightConfigWrapper *wrapper)
{
    if (wrapper == NULL) return;
    free(wrapper);
}

// Java: ((UnmodifiableConfig) value).valueMap() - the Map<String, Object> view of a config table
LIBMATTI_JU_HashMap *LIBMATTI_FML_NightConfigWrapper_ValueMap(LIBMATTI_NC_Config *config)
{
    LIBMATTI_JU_HashMap *map = LIBMATTI_JU_HashMap_New();
    if (config == NULL) return map;

    size_t count = 0;
    LIBMATTI_NC_Value *values = LIBMATTI_NC_Config_ValueMap(config, &count);
    for (size_t i = 0; i < count; i++)
    {
        const char *key = LIBMATTI_NC_Config_KeyAt(config, i);
        if (key == NULL) continue;
        switch (values[i].type)
        {
        case LIBMATTI_NC_VALUE_STRING:
            LIBMATTI_JU_HashMap_Put(map, key, strdup(values[i].string));
            break;
        case LIBMATTI_NC_VALUE_BOOLEAN:
        case LIBMATTI_NC_VALUE_INT:
            LIBMATTI_JU_HashMap_Put(map, key, (void *) (intptr_t) (values[i].boolean | values[i].integer));
            break;
        case LIBMATTI_NC_VALUE_LONG:
        case LIBMATTI_NC_VALUE_DOUBLE:
        case LIBMATTI_NC_VALUE_CONFIG:
        case LIBMATTI_NC_VALUE_LIST:
            // Java keeps the boxed value; the port keeps the NC value handle (owned by the config)
            LIBMATTI_JU_HashMap_Put(map, key, &values[i]);
            break;
        default:
            break;
        }
    }
    return map;
}

// Java: public <T> Optional<T> getConfigElement(String... key)
int LIBMATTI_FML_NightConfigWrapper_GetConfigElement(void *self, const char *const *key, size_t keyCount,
                                                     LIBMATTI_NEOFORGESPI_IConfigurable_Value *out)
{
    LIBMATTI_FML_NightConfigWrapper *wrapper = self;
    LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_Get(wrapper->config, (const char **) key, keyCount);
    if (value == NULL) return 0;

    switch (value->type)
    {
    case LIBMATTI_NC_VALUE_CONFIG:
        // Java: ((UnmodifiableConfig) value).valueMap()
        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_MAP;
        out->map = value->config;
        return 1;
    case LIBMATTI_NC_VALUE_LIST:
        if (value->listCount > 0 && value->list[0].type == LIBMATTI_NC_VALUE_CONFIG)
        {
            LIBMATTI_NEOFORGESPI_InvalidModFileException_New(
                "The configuration path is invalid. I wasn't expecting a multi-object list", wrapper->file);
            return 0;
        }
        // Java returns the raw List; the port only represents a list of strings
        for (size_t i = 0; i < value->listCount; i++)
            if (value->list[i].type != LIBMATTI_NC_VALUE_STRING) return 0;

        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_STRING_LIST;
        out->list = malloc(sizeof(char *) * value->listCount);
        out->listCount = value->listCount;
        for (size_t i = 0; i < value->listCount; i++) out->list[i] = value->list[i].string;
        return 1;
    case LIBMATTI_NC_VALUE_STRING:
        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_STRING;
        out->string = value->string;
        return 1;
    case LIBMATTI_NC_VALUE_BOOLEAN:
        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_BOOL;
        out->boolean = value->boolean;
        return 1;
    case LIBMATTI_NC_VALUE_INT:
        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_INT;
        out->integer = value->integer;
        return 1;
    case LIBMATTI_NC_VALUE_LONG:
        // Java returns the Long; the port's value type only carries an int
        out->type = LIBMATTI_NEOFORGESPI_IConfigurable_INT;
        out->integer = (int) value->longValue;
        return 1;
    default:
        // Java returns the Double as-is
        return 0;
    }
}

// Java: public List<? extends IConfigurable> getConfigList(String... key)
LIBMATTI_NEOFORGESPI_IConfigurable **LIBMATTI_FML_NightConfigWrapper_GetConfigList(void *self, const char *const *key,
                                                                                    size_t keyCount, size_t *count)
{
    LIBMATTI_FML_NightConfigWrapper *wrapper = self;
    LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_Get(wrapper->config, (const char **) key, keyCount);

    if (value != NULL && value->type != LIBMATTI_NC_VALUE_LIST)
    {
        LIBMATTI_NEOFORGESPI_InvalidModFileException_New(
            "The configuration path is invalid. Expecting a collection!", wrapper->file);
        *count = 0;
        return NULL;
    }
    if (value == NULL) // Java: getOrElse(path, ArrayList::new)
    {
        *count = 0;
        return NULL;
    }

    // Java: stream().map(NightConfigWrapper::new).map(cw -> cw.setFile(file)).toList()
    // The wrappers live as long as the mod infos, so they are not released here.
    LIBMATTI_NEOFORGESPI_IConfigurable **result = calloc(value->listCount, sizeof(*result));
    size_t actual = 0;
    for (size_t i = 0; i < value->listCount; i++)
    {
        if (value->list[i].type != LIBMATTI_NC_VALUE_CONFIG) continue;
        LIBMATTI_FML_NightConfigWrapper *nested = LIBMATTI_FML_NightConfigWrapper_New(value->list[i].config);
        LIBMATTI_FML_NightConfigWrapper_SetFile(nested, wrapper->file);
        result[actual++] = LIBMATTI_FML_NightConfigWrapper_AsConfigurable(nested);
    }

    *count = actual;
    return result;
}
