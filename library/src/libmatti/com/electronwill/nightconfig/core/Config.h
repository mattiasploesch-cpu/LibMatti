// Port of com.electronwill.nightconfig.core.Config and UnmodifiableConfig.
// Java keeps the values in a Map<String, Object> with nested Configs, Lists (arrays and arrays of
// tables) and the primitive wrappers; the port keeps the same model with an insertion-ordered
// key/value array (Java's map creator is a LinkedHashMap).

#ifndef MATTICRAFT_NC_CORE_CONFIG_H
#define MATTICRAFT_NC_CORE_CONFIG_H

#include "libmatti/java/time/Temporal.h"

#include <stddef.h>

typedef struct LIBMATTI_NC_Config LIBMATTI_NC_Config;
typedef struct LIBMATTI_NC_Value LIBMATTI_NC_Value;

// Java: the runtime type of a config value
typedef enum
{
    LIBMATTI_NC_VALUE_NULL = 0,
    LIBMATTI_NC_VALUE_CONFIG,
    LIBMATTI_NC_VALUE_LIST,
    LIBMATTI_NC_VALUE_STRING,
    LIBMATTI_NC_VALUE_BOOLEAN,
    LIBMATTI_NC_VALUE_INT,
    LIBMATTI_NC_VALUE_LONG,
    LIBMATTI_NC_VALUE_DOUBLE,
    // Java: the config layer also stores java.time Temporals (TOML dates/times)
    LIBMATTI_NC_VALUE_TEMPORAL
} LIBMATTI_NC_ValueType;

struct LIBMATTI_NC_Value
{
    LIBMATTI_NC_ValueType type;
    char *string;
    int boolean;
    int integer;
    long long longValue;
    double doubleValue;
    // Java: the Temporal of a date/time value
    LIBMATTI_JT_Temporal temporal;
    LIBMATTI_NC_Config *config;
    LIBMATTI_NC_Value *list;
    size_t listCount;
};

struct LIBMATTI_NC_Config
{
    char **keys;
    LIBMATTI_NC_Value *values;
    size_t count;
};

// Java: the value constructors
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfString(const char *value);
// the value takes ownership of the string (avoids the copy of Value_OfString)
LIBMATTI_NC_Value LIBMATTI_NC_Value_OwnString(char *value);
// Java: List.add(value), growing the backing array
void LIBMATTI_NC_Value_ListAppend(LIBMATTI_NC_Value *list, LIBMATTI_NC_Value value);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfBoolean(int value);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfInt(int value);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfLong(long long value);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfDouble(double value);
// Java: a java.time.temporal.Temporal value (LocalDate/LocalTime/LocalDateTime/OffsetDateTime)
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfTemporal(LIBMATTI_JT_Temporal temporal);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfConfig(LIBMATTI_NC_Config *config);
LIBMATTI_NC_Value LIBMATTI_NC_Value_OfList(LIBMATTI_NC_Value *values, size_t count);
void LIBMATTI_NC_Value_Free(LIBMATTI_NC_Value *value);

// Java: public Config()
LIBMATTI_NC_Config *LIBMATTI_NC_Config_New(void);
void LIBMATTI_NC_Config_Free(LIBMATTI_NC_Config *config);

// Java: public Config createSubConfig()
LIBMATTI_NC_Config *LIBMATTI_NC_Config_CreateSubConfig(const LIBMATTI_NC_Config *config);
// Java: public boolean isEmpty()
int LIBMATTI_NC_Config_IsEmpty(const LIBMATTI_NC_Config *config);
// Java: public void clear()
void LIBMATTI_NC_Config_Clear(LIBMATTI_NC_Config *config);

// Java: public Object get(String key)
LIBMATTI_NC_Value *LIBMATTI_NC_Config_GetKey(const LIBMATTI_NC_Config *config, const char *key);
// Java: public Object get(List<String> path)
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Get(const LIBMATTI_NC_Config *config, const char **path, size_t pathLength);
// Java: public boolean contains(List<String> path)
int LIBMATTI_NC_Config_Contains(const LIBMATTI_NC_Config *config, const char **path, size_t pathLength);
// Java: public boolean contains(String key)
int LIBMATTI_NC_Config_ContainsKey(const LIBMATTI_NC_Config *config, const char *key);
// Java: public <T> T set(List<String> path, Object value) - returns the previous value, NULL if none
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Set(LIBMATTI_NC_Config *config, const char **path, size_t pathLength,
                                          const LIBMATTI_NC_Value *value);
// Java: public void add(List<String> path, Object value) - the value is only set when absent
LIBMATTI_NC_Value *LIBMATTI_NC_Config_Add(LIBMATTI_NC_Config *config, const char **path, size_t pathLength,
                                          const LIBMATTI_NC_Value *value);
// Java: public Map<String, Object> valueMap() - the backing entries
LIBMATTI_NC_Value *LIBMATTI_NC_Config_ValueMap(const LIBMATTI_NC_Config *config, size_t *count);
// Java: the keys of valueMap(), in insertion order
const char *LIBMATTI_NC_Config_KeyAt(const LIBMATTI_NC_Config *config, size_t index);
// Java: public Object get(List<String> path) on the last element of an array of tables
LIBMATTI_NC_Value *LIBMATTI_NC_Value_LastOfList(const LIBMATTI_NC_Value *value);

#endif //MATTICRAFT_NC_CORE_CONFIG_H
