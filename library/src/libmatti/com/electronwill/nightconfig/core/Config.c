// Port of com.electronwill.nightconfig.core.Config.

#include "libmatti/com/electronwill/nightconfig/core/Config.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfString(const char *value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_STRING;
    result.string = value != NULL ? strdup(value) : NULL;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OwnString(char *value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_STRING;
    result.string = value;
    return result;
}

void LIBMATTI_NC_Value_ListAppend(LIBMATTI_NC_Value *list, LIBMATTI_NC_Value value)
{
    list->list = realloc(list->list, sizeof(LIBMATTI_NC_Value) * (list->listCount + 1));
    list->list[list->listCount++] = value;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfBoolean(int value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_BOOLEAN;
    result.boolean = value;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfInt(int value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_INT;
    result.integer = value;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfLong(long long value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_LONG;
    result.longValue = value;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfDouble(double value)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_DOUBLE;
    result.doubleValue = value;
    return result;
}

// Java: a java.time.temporal.Temporal value
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfTemporal(LIBMATTI_JT_Temporal temporal)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_TEMPORAL;
    result.temporal = temporal;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfConfig(LIBMATTI_NC_Config *config)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_CONFIG;
    result.config = config;
    return result;
}

LIBMATTI_NC_Value LIBMATTI_NC_Value_OfList(LIBMATTI_NC_Value *values, size_t count)
{
    LIBMATTI_NC_Value result = {0};
    result.type = LIBMATTI_NC_VALUE_LIST;
    result.list = values;
    result.listCount = count;
    return result;
}

void LIBMATTI_NC_Value_Free(LIBMATTI_NC_Value *value)
{
    if (value == NULL) return;

    free(value->string);
    if (value->type == LIBMATTI_NC_VALUE_CONFIG)
    {
        LIBMATTI_NC_Config_Free(value->config);
    }
    else if (value->type == LIBMATTI_NC_VALUE_LIST)
    {
        for (size_t i = 0; i < value->listCount; i++) LIBMATTI_NC_Value_Free(&value->list[i]);
        free(value->list);
    }

    value->string = NULL;
    value->config = NULL;
    value->list = NULL;
    value->listCount = 0;
}

LIBMATTI_NC_Config *LIBMATTI_NC_Config_New(void)
{
    return calloc(1, sizeof(LIBMATTI_NC_Config));
}

void LIBMATTI_NC_Config_Free(LIBMATTI_NC_Config *config)
{
    if (config == NULL) return;

    for (size_t i = 0; i < config->count; i++)
    {
        free(config->keys[i]);
        LIBMATTI_NC_Value_Free(&config->values[i]);
    }
    free(config->keys);
    free(config->values);
    free(config);
}

LIBMATTI_NC_Config *LIBMATTI_NC_Config_CreateSubConfig(const LIBMATTI_NC_Config *config)
{
    (void) config;
    return LIBMATTI_NC_Config_New();
}

int LIBMATTI_NC_Config_IsEmpty(const LIBMATTI_NC_Config *config)
{
    return config->count == 0;
}

void LIBMATTI_NC_Config_Clear(LIBMATTI_NC_Config *config)
{
    for (size_t i = 0; i < config->count; i++)
    {
        free(config->keys[i]);
        LIBMATTI_NC_Value_Free(&config->values[i]);
    }
    free(config->keys);
    free(config->values);
    config->keys = NULL;
    config->values = NULL;
    config->count = 0;
}

// Java: Map.get(key) - the case sensitive key lookup of the TOML format
static long indexOfKey(const LIBMATTI_NC_Config *config, const char *key)
{
    for (size_t i = 0; i < config->count; i++)
        if (strcmp(config->keys[i], key) == 0) return (long) i;
    return -1;
}

LIBMATTI_NC_Value *LIBMATTI_NC_Config_GetKey(const LIBMATTI_NC_Config *config, const char *key)
{
    long index = indexOfKey(config, key);
    return index < 0 ? NULL : &config->values[index];
}

LIBMATTI_NC_Value *LIBMATTI_NC_Value_LastOfList(const LIBMATTI_NC_Value *value)
{
    if (value == NULL || value->type != LIBMATTI_NC_VALUE_LIST || value->listCount == 0) return NULL;
    return &value->list[value->listCount - 1];
}

// Java: UnmodifiableConfig.get(List<String> path)
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Get(const LIBMATTI_NC_Config *config, const char **path, size_t pathLength)
{
    if (config == NULL || pathLength == 0) return NULL;

    const LIBMATTI_NC_Config *current = config;
    for (size_t i = 0; i + 1 < pathLength; i++)
    {
        LIBMATTI_NC_Value *value = LIBMATTI_NC_Config_GetKey(current, path[i]);
        if (value == NULL) return NULL;

        if (value->type == LIBMATTI_NC_VALUE_CONFIG)
        {
            current = value->config;
        }
        else
        {
            // Java: Arrays of tables resolve to their last element
            LIBMATTI_NC_Value *last = LIBMATTI_NC_Value_LastOfList(value);
            if (last == NULL || last->type != LIBMATTI_NC_VALUE_CONFIG) return NULL;
            current = last->config;
        }
    }

    return LIBMATTI_NC_Config_GetKey(current, path[pathLength - 1]);
}

int LIBMATTI_NC_Config_Contains(const LIBMATTI_NC_Config *config, const char **path, size_t pathLength)
{
    return LIBMATTI_NC_Config_Get(config, path, pathLength) != NULL;
}

int LIBMATTI_NC_Config_ContainsKey(const LIBMATTI_NC_Config *config, const char *key)
{
    return indexOfKey(config, key) >= 0;
}

// Java: public <T> T set(String key, Object value)
static LIBMATTI_NC_Value *setKey(LIBMATTI_NC_Config *config, const char *key, const LIBMATTI_NC_Value *value)
{
    long index = indexOfKey(config, key);
    LIBMATTI_NC_Value previous = {0};

    if (index >= 0)
    {
        previous = config->values[index];
        config->values[index] = *value;
        return index >= 0 ? &config->values[index] : NULL;
    }

    config->keys = realloc(config->keys, sizeof(*config->keys) * (config->count + 1));
    config->values = realloc(config->values, sizeof(*config->values) * (config->count + 1));
    config->keys[config->count] = strdup(key);
    config->values[config->count] = *value;
    config->count++;

    (void) previous;
    return NULL;
}

// Java: public <T> T set(List<String> path, Object value)
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Set(LIBMATTI_NC_Config *config, const char **path, size_t pathLength,
                                          const LIBMATTI_NC_Value *value)
{
    if (pathLength == 0) return NULL;

    LIBMATTI_NC_Config *current = config;
    for (size_t i = 0; i + 1 < pathLength; i++)
    {
        LIBMATTI_NC_Value *existing = LIBMATTI_NC_Config_GetKey(current, path[i]);
        if (existing != NULL && existing->type == LIBMATTI_NC_VALUE_CONFIG)
        {
            current = existing->config;
            continue;
        }

        // Java: MERGE removes an incompatible intermediary level and sets a sub config instead
        if (existing != NULL) LIBMATTI_NC_Value_Free(existing);

        LIBMATTI_NC_Value subConfig = LIBMATTI_NC_Value_OfConfig(LIBMATTI_NC_Config_CreateSubConfig(current));
        setKey(current, path[i], &subConfig);
        current = LIBMATTI_NC_Config_GetKey(current, path[i])->config;
    }

    return setKey(current, path[pathLength - 1], value);
}

// Java: public void add(List<String> path, Object value)
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Add(LIBMATTI_NC_Config *config, const char **path, size_t pathLength,
                                          const LIBMATTI_NC_Value *value)
{
    if (LIBMATTI_NC_Config_Contains(config, path, pathLength)) return NULL;
    return LIBMATTI_NC_Config_Set(config, path, pathLength, value);
}

LIBMATTI_NC_Value *LIBMATTI_NC_Config_ValueMap(const LIBMATTI_NC_Config *config, size_t *count)
{
    *count = config->count;
    return config->values;
}

const char *LIBMATTI_NC_Config_KeyAt(const LIBMATTI_NC_Config *config, size_t index)
{
    return config->keys[index];
}
