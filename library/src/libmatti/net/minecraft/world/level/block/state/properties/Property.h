// Port of net.minecraft.world.level.block.state.properties.Property (and the three
// concrete subclasses BooleanProperty, IntegerProperty, EnumProperty).
// Java's generics T extends Comparable<T> collapse to a tagged value: every property
// carries its values as indices into its own possibleValues list, exactly like Java's
// getInternalIndex, so the StateHolder neighbours table can be a plain array.

#ifndef MATTICRAFT_MC_BLOCKSTATE_PROPERTY_H
#define MATTICRAFT_MC_BLOCKSTATE_PROPERTY_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: the three subclasses - the kind tag replaces the type hierarchy
typedef enum
{
    LIBMATTI_MC_Property_KIND_BOOLEAN,
    LIBMATTI_MC_Property_KIND_INTEGER,
    LIBMATTI_MC_Property_KIND_ENUM
} LIBMATTI_MC_Property_Kind;

// Java: T extends Comparable<T> - a value is the index into the property's possibleValues
// list plus the raw payload for the boolean/int cases.
typedef struct LIBMATTI_MC_Property_Value
{
    int index;       // Java: getInternalIndex(value) - also the raw value for IntegerProperty
    int b;           // Java: Boolean - 0/1 (KIND_BOOLEAN only)
    const char *s;   // Java: StringRepresentable.getSerializedName() (KIND_ENUM only)
} LIBMATTI_MC_Property_Value;

// Java: public abstract class Property<T extends Comparable<T>>
typedef struct LIBMATTI_MC_Property
{
    LIBMATTI_MC_Property_Kind kind;
    // Java: private final String name
    char *name;
    // Java: private final Class<T> clazz - the kind tag plus a name for diagnostics
    const char *clazz;
    // Java: abstract List<T> getPossibleValues() - the value list in declaration order
    LIBMATTI_MC_Property_Value *values;
    int valueCount;
    // Java: EnumProperty names map / IntegerProperty min+max
    int min;
    int max;
    // Java: private @Nullable Integer hashCode (cached)
    int hashCode;
} LIBMATTI_MC_Property;

// -----------------------------------------------------------------------
// Property factory + base surface
// -----------------------------------------------------------------------

// Java: BooleanProperty.create(String)
LIBMATTI_MC_Property *LIBMATTI_MC_BooleanProperty_Create(const char *name);
// Java: IntegerProperty.create(String, int, int) - aborts when min < 0 or max <= min
LIBMATTI_MC_Property *LIBMATTI_MC_IntegerProperty_Create(const char *name, int min, int max);
// Java: EnumProperty.create(String, values) - values is an array of serialized names
LIBMATTI_MC_Property *LIBMATTI_MC_EnumProperty_Create(const char *name, const char *const *serialNames, int count);

// Java: public String getName()
const char *LIBMATTI_MC_Property_GetName(const LIBMATTI_MC_Property *property);
// Java: public abstract List<T> getPossibleValues()
const LIBMATTI_MC_Property_Value *LIBMATTI_MC_Property_GetPossibleValues(const LIBMATTI_MC_Property *property, int *count);
// Java: public abstract String getName(T)
const char *LIBMATTI_MC_Property_ValueName(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value);
// Java: public abstract Optional<T> getValue(String) - found = 0 when unparsable
LIBMATTI_MC_Property_Value LIBMATTI_MC_Property_ParseValue(const LIBMATTI_MC_Property *property,
                                                           const char *serialized, int *found);
// Java: public abstract int getInternalIndex(T)
int LIBMATTI_MC_Property_InternalIndex(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value);
// Java: public boolean equals(Object) - clazz equality + name equality
int LIBMATTI_MC_Property_Equals(const LIBMATTI_MC_Property *a, const LIBMATTI_MC_Property *b);
// Java: public int hashCode() - cached
int LIBMATTI_MC_Property_HashCode(LIBMATTI_MC_Property *property);

// Java: the value list index of a value; -1 when the value does not belong to the property
int LIBMATTI_MC_Property_IndexOf(const LIBMATTI_MC_Property *property, LIBMATTI_MC_Property_Value value);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_PROPERTY_H
