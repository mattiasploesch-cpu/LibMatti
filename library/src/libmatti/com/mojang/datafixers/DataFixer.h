// Port of com.mojang.datafixers (the surface the game needs):
// DataFixerUpper with versioned DataFix entries and the Dynamic value tree.
// The generic optics/type rewriting is folded into the version rule the game
// uses: fix(schemaVersion, data) picks the newest fix chain.

#ifndef MATTICRAFT_DFUP_DATAFIXER_H
#define MATTICRAFT_DFUP_DATAFIXER_H

#include <stddef.h>

// ---------------------------------------------------------------------------
// Java: com.mojang.serialization.Dynamic<T> - the value tree the fixes walk.
// The port's Dynamic is an untyped JSON-like tree (the DynamicOps is folded
// into the tree itself).
// ---------------------------------------------------------------------------

typedef enum
{
    LIBMATTI_DFUP_ValueType_EMPTY,
    LIBMATTI_DFUP_ValueType_BOOLEAN,
    LIBMATTI_DFUP_ValueType_INT,
    LIBMATTI_DFUP_ValueType_LONG,
    LIBMATTI_DFUP_ValueType_FLOAT,
    LIBMATTI_DFUP_ValueType_DOUBLE,
    LIBMATTI_DFUP_ValueType_STRING,
    LIBMATTI_DFUP_ValueType_LIST,
    LIBMATTI_DFUP_ValueType_MAP
} LIBMATTI_DFUP_ValueType;

typedef struct LIBMATTI_DFUP_Dynamic LIBMATTI_DFUP_Dynamic;
typedef struct
{
    char *key;
    LIBMATTI_DFUP_Dynamic *value;
} LIBMATTI_DFUP_MapEntry;

struct LIBMATTI_DFUP_Dynamic
{
    LIBMATTI_DFUP_ValueType type;
    int booleanValue;
    int intValue;
    long longValue;
    float floatValue;
    double doubleValue;
    char *stringValue;      // owned
    LIBMATTI_DFUP_Dynamic **list; // owned, size listCount
    size_t listCount;
    LIBMATTI_DFUP_MapEntry *map; // owned entries (key + value), size mapCount
    size_t mapCount;
};

// Java: Dynamic constructor equivalents
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Empty(void);
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Int(int value);
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_String(const char *value);
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Map(void);
// Sets or replaces a map entry (the key is copied)
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Set(LIBMATTI_DFUP_Dynamic *dynamic, const char *key,
                                                 LIBMATTI_DFUP_Dynamic *value);
// Java: Dynamic.get(key) -> Optional - NULL when absent
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Get(const LIBMATTI_DFUP_Dynamic *dynamic, const char *key);
// Java: Dynamic.getOrThrow style accessors
int LIBMATTI_DFUP_Dynamic_AsInt(const LIBMATTI_DFUP_Dynamic *dynamic, int fallback);
const char *LIBMATTI_DFUP_Dynamic_AsString(const LIBMATTI_DFUP_Dynamic *dynamic, const char *fallback);
void LIBMATTI_DFUP_Dynamic_Free(LIBMATTI_DFUP_Dynamic *dynamic);

// ---------------------------------------------------------------------------
// Java: com.mojang.datafixers.DataFix - one fix version
// The port's fix receives the dynamic tree and returns the rewritten tree.
// ---------------------------------------------------------------------------

typedef LIBMATTI_DFUP_Dynamic *(*LIBMATTI_DFUP_Fix)(LIBMATTI_DFUP_Dynamic *data, void *self);

typedef struct
{
    int version;        // Java: the schema version this fix upgrades to
    char *name;         // Java: SCHEMA_NAME / the fix name
    LIBMATTI_DFUP_Fix fix;
    void *self;
} LIBMATTI_DFUP_DataFix;

// ---------------------------------------------------------------------------
// Java: com.mojang.datafixers.DataFixerUpper
// ---------------------------------------------------------------------------

typedef struct
{
    LIBMATTI_DFUP_DataFix *fixes; // sorted by version ascending
    size_t fixCount;
    size_t fixCapacity;
} LIBMATTI_DFUP_DataFixerUpper;

// Java: DataFixerBuilder.addSchema(version, fix) - the port registers one fix
void LIBMATTI_DFUP_DataFixerUpper_Add(LIBMATTI_DFUP_DataFixerUpper *fixer, int version, const char *name,
                                      LIBMATTI_DFUP_Fix fix, void *self);
// Java: DataFixer.build(Util.memoize...) / DataFixerUpper.update(version, input)
// Runs every fix with version > dataVersion, in ascending order.
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_DataFixerUpper_Update(LIBMATTI_DFUP_DataFixerUpper *fixer,
                                                           LIBMATTI_DFUP_Dynamic *input, int dataVersion,
                                                           int currentVersion);
void LIBMATTI_DFUP_DataFixerUpper_Free(LIBMATTI_DFUP_DataFixerUpper *fixer);

// Java: DataFixUtils.updateIfChanged style helper
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_DataFixUtils_UpdateNamed(
    LIBMATTI_DFUP_Dynamic *data, const char *oldName, const char *newName);

#endif //MATTICRAFT_DFUP_DATAFIXER_H
