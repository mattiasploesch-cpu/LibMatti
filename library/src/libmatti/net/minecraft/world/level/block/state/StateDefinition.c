// Port of net.minecraft.world.level.block.state.StateDefinition.

#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Java: static final Pattern NAME_PATTERN = Pattern.compile("^[a-z0-9_]+$")
static int matches_name_pattern(const char *name)
{
    if (*name == '\0')
        return 0;
    for (const char *p = name; *p != '\0'; p++)
    {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_'))
            return 0;
    }
    return 1;
}

// Java: public Builder(O owner)
LIBMATTI_MC_StateDefinition_Builder *LIBMATTI_MC_StateDefinition_Builder_New(void *owner)
{
    LIBMATTI_MC_StateDefinition_Builder *builder = calloc(1, sizeof(LIBMATTI_MC_StateDefinition_Builder));
    builder->owner = owner;
    return builder;
}

// Java: private <T> void validateProperty(Property<T>)
static void validate_property(void *owner, LIBMATTI_MC_StateDefinition_Builder *builder, LIBMATTI_MC_Property *property)
{
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
    const char *name = property->name;
    if (!matches_name_pattern(name))
    {
        // Java: throw new IllegalArgumentException(owner + " has invalidly named property: " + s)
        LIBMATTI_ML_Logger_Error(logger, NULL, "has invalidly named property: {}", name);
        exit(1);
    }
    if (property->valueCount <= 1)
    {
        // Java: throw new IllegalArgumentException(owner + " attempted use property " + s + " with <= 1 possible values")
        LIBMATTI_ML_Logger_Error(logger, NULL, "attempted use property {} with <= 1 possible values", name);
        exit(1);
    }
    for (int i = 0; i < property->valueCount; i++)
    {
        if (!matches_name_pattern(LIBMATTI_MC_Property_ValueName(property, property->values[i])))
        {
            // Java: throw new IllegalArgumentException(owner + " has property: " + s + " with invalidly named value: " + s1)
            LIBMATTI_ML_Logger_Error(logger, NULL, "has property: {} with invalidly named value", name);
            exit(1);
        }
    }
    for (int i = 0; i < builder->propertyCount; i++)
    {
        if (strcmp(builder->properties[i]->name, name) == 0)
        {
            // Java: throw new IllegalArgumentException(owner + " has duplicate property: " + s)
            LIBMATTI_ML_Logger_Error(logger, NULL, "has duplicate property: {}", name);
            exit(1);
        }
    }
}

// Java: public StateDefinition.Builder<O, S> add(Property<?>...)
LIBMATTI_MC_StateDefinition_Builder *LIBMATTI_MC_StateDefinition_Builder_Add(LIBMATTI_MC_StateDefinition_Builder *builder,
                                                                             LIBMATTI_MC_Property *property)
{
    validate_property(builder->owner, builder, property);
    if (builder->propertyCount == builder->capacity)
    {
        builder->capacity = builder->capacity > 0 ? builder->capacity * 2 : 4;
        builder->properties = realloc(builder->properties, sizeof(LIBMATTI_MC_Property *) * builder->capacity);
    }
    // Java: properties.put(property.getName(), property) - the port keeps the insertion
    // order (Java sorts the map by name when copying; the state table product order then
    // follows the sorted property order, which the port reproduces below at create())
    builder->properties[builder->propertyCount++] = property;
    return builder;
}

// Java: private static int the sorted property order - ImmutableSortedMap sorts by name
static int compare_property_names(const void *a, const void *b)
{
    const LIBMATTI_MC_Property *pa = *(const LIBMATTI_MC_Property *const *) a;
    const LIBMATTI_MC_Property *pb = *(const LIBMATTI_MC_Property *const *) b;
    return strcmp(pa->name, pb->name);
}

// Java: public StateDefinition<O, S> create(Function<O, S>, Factory<O, S>)
LIBMATTI_MC_StateDefinition *LIBMATTI_MC_StateDefinition_Builder_Create(LIBMATTI_MC_StateDefinition_Builder *builder)
{
    LIBMATTI_MC_StateDefinition *definition = calloc(1, sizeof(LIBMATTI_MC_StateDefinition));
    definition->owner = builder->owner;
    definition->propertyCount = builder->propertyCount;
    definition->properties = malloc(sizeof(LIBMATTI_MC_Property *) * (builder->propertyCount > 0 ? builder->propertyCount : 1));
    memcpy(definition->properties, builder->properties, sizeof(LIBMATTI_MC_Property *) * builder->propertyCount);
    // Java: ImmutableSortedMap.copyOf(properties) - the sorted property order drives the table
    qsort(definition->properties, definition->propertyCount, sizeof(LIBMATTI_MC_Property *), compare_property_names);

    // Java: the state count is the product of every property's value count
    int stateCount = 1;
    for (int i = 0; i < definition->propertyCount; i++)
        stateCount *= definition->properties[i]->valueCount;
    definition->stateCount = stateCount;
    definition->states = malloc(sizeof(LIBMATTI_MC_StateHolder *) * (stateCount > 0 ? stateCount : 1));

    // Java: the cartesian product in declaration order - the last property varies fastest
    int *cursor = calloc(definition->propertyCount > 0 ? definition->propertyCount : 1, sizeof(int));
    for (int s = 0; s < stateCount; s++)
    {
        LIBMATTI_MC_StateHolder *state = calloc(1, sizeof(LIBMATTI_MC_StateHolder));
        state->owner = builder->owner;
        state->valueCount = definition->propertyCount;
        state->properties = definition->properties;
        state->values = malloc(sizeof(LIBMATTI_MC_Property_Value) * (definition->propertyCount > 0 ? definition->propertyCount : 1));
        for (int i = 0; i < definition->propertyCount; i++)
            state->values[i] = definition->properties[i]->values[cursor[i]];
        definition->states[s] = state;

        // odometer increment: last property fastest
        for (int i = definition->propertyCount - 1; i >= 0; i--)
        {
            if (++cursor[i] < definition->properties[i]->valueCount)
                break;
            cursor[i] = 0;
        }
    }
    free(cursor);

    // Java: for (S s : list) s.populateNeighbours(map)
    for (int s = 0; s < stateCount; s++)
        LIBMATTI_MC_StateHolder_PopulateNeighbours(definition->states[s], definition->states, stateCount);

    return definition;
}

// Java: public ImmutableList<S> getPossibleStates()
LIBMATTI_MC_StateHolder **LIBMATTI_MC_StateDefinition_GetPossibleStates(const LIBMATTI_MC_StateDefinition *definition,
                                                                        int *count)
{
    if (count != NULL)
        *count = definition->stateCount;
    return definition->states;
}

// Java: public S any()
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateDefinition_Any(const LIBMATTI_MC_StateDefinition *definition)
{
    return definition->states[0];
}

// Java: public O getOwner()
void *LIBMATTI_MC_StateDefinition_GetOwner(const LIBMATTI_MC_StateDefinition *definition)
{
    return definition->owner;
}

// Java: public Collection<Property<?>> getProperties()
LIBMATTI_MC_Property **LIBMATTI_MC_StateDefinition_GetProperties(const LIBMATTI_MC_StateDefinition *definition,
                                                                 int *count)
{
    if (count != NULL)
        *count = definition->propertyCount;
    return definition->properties;
}

// Java: public Property<?> getProperty(String)
LIBMATTI_MC_Property *LIBMATTI_MC_StateDefinition_GetProperty(const LIBMATTI_MC_StateDefinition *definition,
                                                              const char *name)
{
    for (int i = 0; i < definition->propertyCount; i++)
    {
        if (strcmp(definition->properties[i]->name, name) == 0)
            return definition->properties[i];
    }
    return NULL;
}
