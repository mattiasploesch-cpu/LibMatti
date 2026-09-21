// Port of net.minecraft.world.level.block.state.StateHolder (one state in the block's
// state table). Java keys the values map by property identity; the C port does the same
// with a pointer array ordered like the definition's property list, so the neighbours
// table is a flat array of state pointers per property.

#ifndef MATTICRAFT_MC_BLOCKSTATE_STATEHOLDER_H
#define MATTICRAFT_MC_BLOCKSTATE_STATEHOLDER_H

#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public abstract class StateHolder<O, S> - the port is one struct shared by
// BlockState (the S) with the owner back-link
typedef struct LIBMATTI_MC_StateHolder
{
    // Java: protected final O owner - the Block
    void *owner;
    // Java: private final Reference2ObjectArrayMap<Property<?>, Comparable<?>> values
    LIBMATTI_MC_Property **properties;
    LIBMATTI_MC_Property_Value *values;
    int valueCount;
    // Java: private Map<Property<?>, S[]> neighbours - populated by populateNeighbours;
    // neighbourIndex[property][valueIndex] -> the state with that property changed
    struct LIBMATTI_MC_StateHolder ***neighbours;
} LIBMATTI_MC_StateHolder;

// Java: protected StateHolder(O owner, values) - the StateDefinition drives creation
void LIBMATTI_MC_StateHolder_Init(LIBMATTI_MC_StateHolder *state, void *owner,
                                  LIBMATTI_MC_Property **properties, LIBMATTI_MC_Property_Value *values,
                                  int valueCount);

// Java: public boolean hasProperty(Property<?>)
int LIBMATTI_MC_StateHolder_HasProperty(const LIBMATTI_MC_StateHolder *state, const LIBMATTI_MC_Property *property);
// Java: public <T> T getValue(Property<T>) - aborts on an unknown property (Java throws)
LIBMATTI_MC_Property_Value LIBMATTI_MC_StateHolder_GetValue(const LIBMATTI_MC_StateHolder *state,
                                                            const LIBMATTI_MC_Property *property);
// Java: public Optional<T> getOptionalValue(Property<T>) - found = 0 when unknown
LIBMATTI_MC_Property_Value LIBMATTI_MC_StateHolder_GetOptionalValue(const LIBMATTI_MC_StateHolder *state,
                                                                    const LIBMATTI_MC_Property *property, int *found);
// Java: public <T, V extends T> S setValue(Property<T>, V) - returns the neighbour state
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_SetValue(LIBMATTI_MC_StateHolder *state,
                                                          const LIBMATTI_MC_Property *property,
                                                          LIBMATTI_MC_Property_Value value);
// Java: public <T, V extends T> S trySetValue(Property<T>, V) - this when the property is absent
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_TrySetValue(LIBMATTI_MC_StateHolder *state,
                                                             const LIBMATTI_MC_Property *property,
                                                             LIBMATTI_MC_Property_Value value);
// Java: public <T> S cycle(Property<T>)
LIBMATTI_MC_StateHolder *LIBMATTI_MC_StateHolder_Cycle(LIBMATTI_MC_StateHolder *state,
                                                       LIBMATTI_MC_Property *property);

// Java: public Collection<Property<?>> getProperties()
LIBMATTI_MC_Property **LIBMATTI_MC_StateHolder_GetProperties(const LIBMATTI_MC_StateHolder *state, int *count);
// Java: public Map<Property<?>, Comparable<?>> getValues()
const LIBMATTI_MC_Property_Value *LIBMATTI_MC_StateHolder_GetValues(const LIBMATTI_MC_StateHolder *state, int *count);

// Java: void populateNeighbours(Map<Map<Property<?>, Comparable<?>>, S>) - the StateDefinition
// resolves the value-table map into the per-property neighbour arrays
void LIBMATTI_MC_StateHolder_PopulateNeighbours(LIBMATTI_MC_StateHolder *state,
                                                struct LIBMATTI_MC_StateHolder **states, int stateCount);

// Java: public String toString() - "block[property=value,...]" into a caller buffer
void LIBMATTI_MC_StateHolder_ToString(const LIBMATTI_MC_StateHolder *state, char *buffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_STATEHOLDER_H
