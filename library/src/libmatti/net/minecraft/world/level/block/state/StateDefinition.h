// Port of net.minecraft.world.level.block.state.StateDefinition (the per-block state
// table). Java builds the cartesian product of every property's possible values in the
// property declaration order; the C port keeps the same table layout so the state index
// arithmetic works like Java's, and wires the neighbours through populateNeighbours.

#ifndef MATTICRAFT_MC_BLOCKSTATE_STATEDEFINITION_H
#define MATTICRAFT_MC_BLOCKSTATE_STATEDEFINITION_H

#include "libmatti/net/minecraft/world/level/block/state/StateHolder.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class StateDefinition<O, S>
typedef struct LIBMATTI_MC_StateDefinition
{
    // Java: private final O owner
    void *owner;
    // Java: private final ImmutableSortedMap<String, Property<?>> propertiesByName
    LIBMATTI_MC_Property **properties;
    int propertyCount;
    // Java: private final ImmutableList<S> states (the cartesian product in declaration order)
    LIBMATTI_MC_StateHolder **states;
    int stateCount;
} LIBMATTI_MC_StateDefinition;

// Java: public static class Builder<O, S>
typedef struct LIBMATTI_MC_StateDefinition_Builder
{
    void *owner;
    LIBMATTI_MC_Property **properties;
    int propertyCount;
    int capacity;
} LIBMATTI_MC_StateDefinition_Builder;

// Java: public Builder(O owner)
LIBMATTI_MC_StateDefinition_Builder *LIBMATTI_MC_StateDefinition_Builder_New(void *owner);
// Java: public StateDefinition.Builder<O, S> add(Property<?>...) - validates every property
LIBMATTI_MC_StateDefinition_Builder *LIBMATTI_MC_StateDefinition_Builder_Add(LIBMATTI_MC_StateDefinition_Builder *builder,
                                                                             LIBMATTI_MC_Property *property);
// Java: public StateDefinition<O, S> create(...) - builds the state table
LIBMATTI_MC_StateDefinition *LIBMATTI_MC_StateDefinition_Builder_Create(LIBMATTI_MC_StateDefinition_Builder *builder);

// Java: public ImmutableList<S> getPossibleStates()
LIBMATTI_MC_StateHolder **LIBMATTI_MC_StateDefinition_GetPossibleStates(const LIBMATTI_MC_StateDefinition *definition,
                                                                        int *count);
// Java: public S any() - the first state
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateDefinition_Any(const LIBMATTI_MC_StateDefinition *definition);
// Java: public O getOwner()
void *LIBMATTI_MC_StateDefinition_GetOwner(const LIBMATTI_MC_StateDefinition *definition);
// Java: public Collection<Property<?>> getProperties()
LIBMATTI_MC_Property **LIBMATTI_MC_StateDefinition_GetProperties(const LIBMATTI_MC_StateDefinition *definition,
                                                                 int *count);
// Java: public Property<?> getProperty(String) - NULL when unknown
LIBMATTI_MC_Property *LIBMATTI_MC_StateDefinition_GetProperty(const LIBMATTI_MC_StateDefinition *definition,
                                                              const char *name);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_STATEDEFINITION_H
