// Port of net.minecraft.world.level.block.state.properties.Property and the three
// concrete subclasses.

#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final List<Boolean> VALUES = List.of(true, false)
// TRUE_INDEX = 0, FALSE_INDEX = 1

// Java: public String getName() is inherited; the factories fill the shared struct

static LIBMATTI_MC_Property *alloc_property(const char *name, LIBMATTI_MC_Property_Kind kind, const char *clazz)
{
    LIBMATTI_MC_Property *property = calloc(1, sizeof(LIBMATTI_MC_Property));
    property->kind = kind;
    property->name = strdup(name);
    property->clazz = clazz;
    return property;
}

// Java: private BooleanProperty(String) / public static BooleanProperty create(String)
LIBMATTI_MC_Property *LIBMATTI_MC_BooleanProperty_Create(const char *name)
{
    LIBMATTI_MC_Property *property = alloc_property(name, LIBMATTI_MC_Property_KIND_BOOLEAN, "Boolean");
    // Java: List.of(true, false) - true first, false second
    property->valueCount = 2;
    property->values = malloc(sizeof(LIBMATTI_MC_Property_Value) * 2);
    property->values[0] = (LIBMATTI_MC_Property_Value){0, 1, "true"};
    property->values[1] = (LIBMATTI_MC_Property_Value){1, 0, "false"};
    return property;
}

// Java: private IntegerProperty(String, int, int)
LIBMATTI_MC_Property *LIBMATTI_MC_IntegerProperty_Create(const char *name, int min, int max)
{
    if (min < 0)
    {
        // Java: throw new IllegalArgumentException("Min value of " + name + " must be 0 or greater")
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Min value of {} must be 0 or greater", name);
        exit(1);
    }
    if (max <= min)
    {
        // Java: throw new IllegalArgumentException("Max value of ... must be greater than min")
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Max value of {} must be greater than min", name);
        exit(1);
    }
    LIBMATTI_MC_Property *property = alloc_property(name, LIBMATTI_MC_Property_KIND_INTEGER, "Integer");
    property->min = min;
    property->max = max;
    // Java: IntImmutableList.toList(IntStream.range(min, max + 1))
    property->valueCount = max - min + 1;
    property->values = malloc(sizeof(LIBMATTI_MC_Property_Value) * property->valueCount);
    for (int i = 0; i < property->valueCount; i++)
    {
        int value = min + i;
        char *serialized = malloc(16);
        snprintf(serialized, 16, "%d", value);
        property->values[i] = (LIBMATTI_MC_Property_Value){value, value, serialized};
    }
    return property;
}

// Java: private EnumProperty(String, Class, List)
LIBMATTI_MC_Property *LIBMATTI_MC_EnumProperty_Create(const char *name, const char *const *serialNames, int count)
{
    if (count <= 0)
    {
        // Java: throw new IllegalArgumentException("Trying to make empty EnumProperty '" + name + "'")
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Trying to make empty EnumProperty '{}'", name);
        exit(1);
    }
    LIBMATTI_MC_Property *property = alloc_property(name, LIBMATTI_MC_Property_KIND_ENUM, "Enum");
    property->valueCount = count;
    property->values = malloc(sizeof(LIBMATTI_MC_Property_Value) * count);
    for (int i = 0; i < count; i++)
        property->values[i] = (LIBMATTI_MC_Property_Value){i, 0, serialNames[i]};
    return property;
}

// Java: public String getName()
const char *LIBMATTI_MC_Property_GetName(const LIBMATTI_MC_Property *property)
{
    return property->name;
}

// Java: public abstract List<T> getPossibleValues()
const LIBMATTI_MC_Property_Value *LIBMATTI_MC_Property_GetPossibleValues(const LIBMATTI_MC_Property *property, int *count)
{
    if (count != NULL)
        *count = property->valueCount;
    return property->values;
}

// Java: public abstract String getName(T)
const char *LIBMATTI_MC_Property_ValueName(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value)
{
    (void) property;
    switch (property->kind)
    {
    case LIBMATTI_MC_Property_KIND_BOOLEAN:
        // Java: Boolean.toString()
        return value.b ? "true" : "false";
    case LIBMATTI_MC_Property_KIND_INTEGER:
    {
        // Java: Integer.toString()
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value.index);
        return buffer;
    }
    case LIBMATTI_MC_Property_KIND_ENUM:
        // Java: T.getSerializedName()
        return value.s;
    }
    return "";
}

// Java: public abstract Optional<T> getValue(String)
LIBMATTI_MC_Property_Value LIBMATTI_MC_Property_ParseValue(const LIBMATTI_MC_Property *property,
                                                           const char *serialized, int *found)
{
    if (found != NULL)
        *found = 0;
    switch (property->kind)
    {
    case LIBMATTI_MC_Property_KIND_BOOLEAN:
        // Java: switch (value): "true" -> true, "false" -> false, default -> empty
        if (strcmp(serialized, "true") == 0)
        {
            if (found != NULL) *found = 1;
            return property->values[0];
        }
        if (strcmp(serialized, "false") == 0)
        {
            if (found != NULL) *found = 1;
            return property->values[1];
        }
        break;
    case LIBMATTI_MC_Property_KIND_INTEGER:
    {
        // Java: Integer.parseInt with the range check
        char *end = NULL;
        long parsed = strtol(serialized, &end, 10);
        if (end != serialized && *end == '\0' && parsed >= property->min && parsed <= property->max)
        {
            if (found != NULL) *found = 1;
            return property->values[parsed - property->min];
        }
        break;
    }
    case LIBMATTI_MC_Property_KIND_ENUM:
        // Java: Optional.ofNullable(names.get(value))
        for (int i = 0; i < property->valueCount; i++)
        {
            if (strcmp(property->values[i].s, serialized) == 0)
            {
                if (found != NULL) *found = 1;
                return property->values[i];
            }
        }
        break;
    }
    return (LIBMATTI_MC_Property_Value){-1, 0, NULL};
}

// Java: public abstract int getInternalIndex(T)
int LIBMATTI_MC_Property_InternalIndex(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value)
{
    switch (property->kind)
    {
    case LIBMATTI_MC_Property_KIND_BOOLEAN:
        // Java: value ? 0 : 1
        return value.b ? 0 : 1;
    case LIBMATTI_MC_Property_KIND_INTEGER:
        // Java: value <= max ? value - min : -1
        return value.index <= property->max ? value.index - property->min : -1;
    case LIBMATTI_MC_Property_KIND_ENUM:
        // Java: ordinalToIndex[ordinal] - the port stores the index directly
        return value.index;
    }
    return -1;
}

// Java: public boolean equals(Object) - clazz + name
int LIBMATTI_MC_Property_Equals(const LIBMATTI_MC_Property *a, const LIBMATTI_MC_Property *b)
{
    if (a == b)
        return 1;
    if (a == NULL || b == NULL)
        return 0;
    if (a->kind != b->kind || strcmp(a->name, b->name) != 0)
        return 0;
    // Java: IntegerProperty/EnumProperty add the values equality
    if (a->kind == LIBMATTI_MC_Property_KIND_BOOLEAN)
        return 1;
    if (a->valueCount != b->valueCount)
        return 0;
    for (int i = 0; i < a->valueCount; i++)
    {
        if (a->values[i].index != b->values[i].index)
            return 0;
        if (a->values[i].s != NULL && b->values[i].s != NULL && strcmp(a->values[i].s, b->values[i].s) != 0)
            return 0;
    }
    return 1;
}

// Java: public int generateHashCode() - 31 * clazz.hashCode + name.hashCode (+ values)
int LIBMATTI_MC_Property_HashCode(LIBMATTI_MC_Property *property)
{
    if (property->hashCode != 0)
        return property->hashCode;
    int hash = 31 * (int) property->kind;
    for (const char *p = property->name; *p != '\0'; p++)
        hash = 31 * hash + *p;
    if (property->kind != LIBMATTI_MC_Property_KIND_BOOLEAN)
        hash = 31 * hash + property->valueCount;
    property->hashCode = hash;
    return hash;
}

// Java: the index of the value in getPossibleValues(); -1 when it does not belong
int LIBMATTI_MC_Property_IndexOf(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value)
{
    int internal = LIBMATTI_MC_Property_InternalIndex(property, value);
    if (internal < 0 || internal >= property->valueCount)
        return -1;
    // The value table is indexed exactly like the internal index in every subclass
    return internal;
}
