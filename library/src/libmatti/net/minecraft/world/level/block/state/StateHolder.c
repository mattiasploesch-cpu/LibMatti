// Port of net.minecraft.world.level.block.state.StateHolder.

#include "libmatti/net/minecraft/world/level/block/state/StateHolder.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void LIBMATTI_MC_StateHolder_Init(LIBMATTI_MC_StateHolder *state, void *owner,
                                  LIBMATTI_MC_Property **properties, LIBMATTI_MC_Property_Value *values,
                                  int valueCount)
{
    state->owner = owner;
    state->properties = properties;
    state->values = values;
    state->valueCount = valueCount;
    state->neighbours = NULL;
}

// Java: private <T> T getNullableValue(Property<T>) with the "does not exist" throw
static int find_value_index(const LIBMATTI_MC_StateHolder *state, const LIBMATTI_MC_Property *property)
{
    for (int i = 0; i < state->valueCount; i++)
    {
        if (state->properties[i] == property)
            return i;
    }
    return -1;
}

// Java: public boolean hasProperty(Property<?>)
int LIBMATTI_MC_StateHolder_HasProperty(const LIBMATTI_MC_StateHolder *state, const LIBMATTI_MC_Property *property)
{
    return find_value_index(state, property) >= 0;
}

// Java: public <T> T getValue(Property<T>)
LIBMATTI_MC_Property_Value LIBMATTI_MC_StateHolder_GetValue(const LIBMATTI_MC_StateHolder *state,
                                                            const LIBMATTI_MC_Property *property)
{
    int index = find_value_index(state, property);
    if (index < 0)
    {
        // Java: throw new IllegalArgumentException("Cannot get property " + p + " as it does not exist in " + owner)
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Cannot get property {} as it does not exist",
                                 property->name);
        exit(1);
    }
    return state->values[index];
}

// Java: public Optional<T> getOptionalValue(Property<T>)
LIBMATTI_MC_Property_Value LIBMATTI_MC_StateHolder_GetOptionalValue(const LIBMATTI_MC_StateHolder *state,
                                                                    const LIBMATTI_MC_Property *property, int *found)
{
    int index = find_value_index(state, property);
    if (index < 0)
    {
        if (found != NULL)
            *found = 0;
        return (LIBMATTI_MC_Property_Value){-1, 0, NULL};
    }
    if (found != NULL)
        *found = 1;
    return state->values[index];
}

// Java: public <T, V extends T> S setValue(Property<T>, V) - the neighbour lookup
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_SetValue(LIBMATTI_MC_StateHolder *state,
                                                          const LIBMATTI_MC_Property *property,
                                                          LIBMATTI_MC_Property_Value value)
{
    int index = find_value_index(state, property);
    if (index < 0)
    {
        // Java: throw new IllegalArgumentException("Cannot set property " + p + " as it does not exist in " + owner)
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Cannot set property {} as it does not exist",
                                 property->name);
        exit(1);
    }

    LIBMATTI_MC_Property_Value current = state->values[index];
    if (current.index == value.index && current.b == value.b
        && (current.s == value.s || (current.s != NULL && value.s != NULL && strcmp(current.s, value.s) == 0)))
        return state;

    int internal = LIBMATTI_MC_Property_InternalIndex(property, value);
    if (internal < 0)
    {
        // Java: throw new IllegalArgumentException("Cannot set property " + p + " to " + v + " ... not an allowed value")
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Cannot set property {} to {}, it is not an allowed value",
                                 property->name, LIBMATTI_MC_Property_ValueName(property, value));
        exit(1);
    }
    return state->neighbours[index][internal];
}

// Java: public <T, V extends T> S trySetValue(Property<T>, V)
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_TrySetValue(LIBMATTI_MC_StateHolder *state,
                                                             const LIBMATTI_MC_Property *property,
                                                             LIBMATTI_MC_Property_Value value)
{
    if (!LIBMATTI_MC_StateHolder_HasProperty(state, property))
        return state;
    return LIBMATTI_MC_StateHolder_SetValue(state, property, value);
}

// Java: protected static <T> T findNextInCollection(List<T>, T)
static LIBMATTI_MC_Property_Value find_next(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value current)
{
    int index = LIBMATTI_MC_Property_IndexOf(property, current) + 1;
    return index == property->valueCount ? property->values[0] : property->values[index];
}

// Java: public <T> S cycle(Property<T>)
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_Cycle(LIBMATTI_MC_StateHolder *state, LIBMATTI_MC_Property *property)
{
    return LIBMATTI_MC_StateHolder_SetValue(state, property, find_next(property, LIBMATTI_MC_StateHolder_GetValue(state, property)));
}

// Java: public Collection<Property<?>> getProperties()
LIBMATTI_MC_Property **LIBMATTI_MC_StateHolder_GetProperties(const LIBMATTI_MC_StateHolder *state, int *count)
{
    if (count != NULL)
        *count = state->valueCount;
    return state->properties;
}

// Java: public Map<Property<?>, Comparable<?>> getValues()
const LIBMATTI_MC_Property_Value *LIBMATTI_MC_StateHolder_GetValues(const LIBMATTI_MC_StateHolder *state, int *count)
{
    if (count != NULL)
        *count = state->valueCount;
    return state->values;
}

// Java: void populateNeighbours(Map<Map<Property<?>, Comparable<?>>, S>)
// The StateDefinition passes its full state list; the port matches the neighbour states
// by comparing every property value pair.
void LIBMATTI_MC_StateHolder_PopulateNeighbours(LIBMATTI_MC_StateHolder *state,
                                                LIBMATTI_MC_StateHolder **states, int stateCount)
{
    if (state->neighbours != NULL)
        return;

    state->neighbours = malloc(sizeof(LIBMATTI_MC_StateHolder **) * state->valueCount);
    for (int p = 0; p < state->valueCount; p++)
    {
        LIBMATTI_MC_Property *property = state->properties[p];
        state->neighbours[p] = malloc(sizeof(LIBMATTI_MC_StateHolder *) * property->valueCount);
        for (int v = 0; v < property->valueCount; v++)
        {
            // Java: makeNeighbourValues(property, value) - the same values but this property
            // changed; the port scans the state table for the match
            LIBMATTI_MC_StateHolder *match = state;
            for (int s = 0; s < stateCount; s++)
            {
                LIBMATTI_MC_StateHolder *candidate = states[s];
                int same = candidate->valueCount == state->valueCount;
                for (int i = 0; same && i < state->valueCount; i++)
                {
                    LIBMATTI_MC_Property_Value expected = i == p ? property->values[v] : state->values[i];
                    LIBMATTI_MC_Property_Value actual = candidate->values[i];
                    if (candidate->properties[i] != state->properties[i]
                        || actual.index != expected.index || actual.b != expected.b)
                    {
                        same = 0;
                        break;
                    }
                    if (actual.s != NULL && expected.s != NULL && strcmp(actual.s, expected.s) != 0)
                    {
                        same = 0;
                        break;
                    }
                }
                if (same)
                {
                    match = candidate;
                    break;
                }
            }
            state->neighbours[p][v] = match;
        }
    }
}

// Java: public String toString() - "owner[property=value,...]"
void LIBMATTI_MC_StateHolder_ToString(const LIBMATTI_MC_StateHolder *state, char *buffer, size_t bufferSize)
{
    size_t offset = (size_t) snprintf(buffer, bufferSize, "%p", state->owner);
    if (state->valueCount == 0)
        return;
    if (offset < bufferSize - 1)
        buffer[offset++] = '[';
    for (int i = 0; i < state->valueCount; i++)
    {
        LIBMATTI_MC_Property *property = state->properties[i];
        if (offset >= bufferSize - 1)
            break;
        offset += (size_t) snprintf(buffer + offset, bufferSize - offset, "%s=%s", property->name,
                                    LIBMATTI_MC_Property_ValueName(property, state->values[i]));
        if (i < state->valueCount - 1 && offset < bufferSize - 1)
            buffer[offset++] = ',';
    }
    if (offset < bufferSize - 1)
        buffer[offset++] = ']';
    buffer[offset] = '\0';
}
