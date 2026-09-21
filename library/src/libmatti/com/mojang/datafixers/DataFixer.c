// Port of com.mojang.datafixers.DataFixerUpper (the versioned update chain)
// and the Dynamic value tree the fixes walk.

#include "libmatti/com/mojang/datafixers/DataFixer.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Dynamic
// ---------------------------------------------------------------------------

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Empty(void)
{
    return calloc(1, sizeof(LIBMATTI_DFUP_Dynamic));
}

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Int(int value)
{
    LIBMATTI_DFUP_Dynamic *dynamic = LIBMATTI_DFUP_Dynamic_Empty();
    dynamic->type = LIBMATTI_DFUP_ValueType_INT;
    dynamic->intValue = value;
    return dynamic;
}

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_String(const char *value)
{
    LIBMATTI_DFUP_Dynamic *dynamic = LIBMATTI_DFUP_Dynamic_Empty();
    dynamic->type = LIBMATTI_DFUP_ValueType_STRING;
    dynamic->stringValue = strdup(value);
    return dynamic;
}

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Map(void)
{
    LIBMATTI_DFUP_Dynamic *dynamic = LIBMATTI_DFUP_Dynamic_Empty();
    dynamic->type = LIBMATTI_DFUP_ValueType_MAP;
    return dynamic;
}

void LIBMATTI_DFUP_Dynamic_Free(LIBMATTI_DFUP_Dynamic *dynamic)
{
    if (dynamic == NULL)
        return;
    free(dynamic->stringValue);
    for (size_t i = 0; i < dynamic->listCount; i++)
        LIBMATTI_DFUP_Dynamic_Free(dynamic->list[i]);
    free(dynamic->list);
    for (size_t i = 0; i < dynamic->mapCount; i++)
    {
        free(dynamic->map[i].key);
        LIBMATTI_DFUP_Dynamic_Free(dynamic->map[i].value);
    }
    free(dynamic->map);
    free(dynamic);
}

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Set(LIBMATTI_DFUP_Dynamic *dynamic, const char *key,
                                                 LIBMATTI_DFUP_Dynamic *value)
{
    // Java: Dynamic.set(key, value)
    for (size_t i = 0; i < dynamic->mapCount; i++)
    {
        if (strcmp(dynamic->map[i].key, key) == 0)
        {
            LIBMATTI_DFUP_Dynamic_Free(dynamic->map[i].value);
            dynamic->map[i].value = value;
            return dynamic;
        }
    }
    dynamic->map = realloc(dynamic->map, sizeof(LIBMATTI_DFUP_MapEntry) * (dynamic->mapCount + 1));
    dynamic->map[dynamic->mapCount].key = strdup(key);
    dynamic->map[dynamic->mapCount].value = value;
    dynamic->mapCount++;
    return dynamic;
}

LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_Dynamic_Get(const LIBMATTI_DFUP_Dynamic *dynamic, const char *key)
{
    // Java: Dynamic.get(key) -> Optional
    for (size_t i = 0; i < dynamic->mapCount; i++)
        if (strcmp(dynamic->map[i].key, key) == 0)
            return dynamic->map[i].value;
    return NULL;
}

int LIBMATTI_DFUP_Dynamic_AsInt(const LIBMATTI_DFUP_Dynamic *dynamic, int fallback)
{
    switch (dynamic->type)
    {
        case LIBMATTI_DFUP_ValueType_INT:
            return dynamic->intValue;
        case LIBMATTI_DFUP_ValueType_LONG:
            return (int) dynamic->longValue;
        case LIBMATTI_DFUP_ValueType_FLOAT:
            return (int) dynamic->floatValue;
        case LIBMATTI_DFUP_ValueType_DOUBLE:
            return (int) dynamic->doubleValue;
        default:
            return fallback;
    }
}

const char *LIBMATTI_DFUP_Dynamic_AsString(const LIBMATTI_DFUP_Dynamic *dynamic, const char *fallback)
{
    return dynamic->type == LIBMATTI_DFUP_ValueType_STRING ? dynamic->stringValue : fallback;
}

// ---------------------------------------------------------------------------
// DataFixerUpper
// ---------------------------------------------------------------------------

void LIBMATTI_DFUP_DataFixerUpper_Add(LIBMATTI_DFUP_DataFixerUpper *fixer, int version, const char *name,
                                      LIBMATTI_DFUP_Fix fix, void *self)
{
    if (fixer->fixCount == fixer->fixCapacity)
    {
        fixer->fixCapacity = fixer->fixCapacity == 0 ? 8 : fixer->fixCapacity * 2;
        fixer->fixes = realloc(fixer->fixes, sizeof(LIBMATTI_DFUP_DataFix) * fixer->fixCapacity);
    }
    fixer->fixes[fixer->fixCount].version = version;
    fixer->fixes[fixer->fixCount].name = strdup(name);
    fixer->fixes[fixer->fixCount].fix = fix;
    fixer->fixes[fixer->fixCount].self = self;
    fixer->fixCount++;
}

// Java: DataFixerUpper.update - the fixes run in ascending version order
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_DataFixerUpper_Update(LIBMATTI_DFUP_DataFixerUpper *fixer,
                                                           LIBMATTI_DFUP_Dynamic *input, int dataVersion,
                                                           int currentVersion)
{
    // Java: if (dataVersion >= currentVersion) return input;
    if (dataVersion >= currentVersion)
        return input;

    LIBMATTI_DFUP_Dynamic *data = input;
    for (size_t i = 0; i < fixer->fixCount; i++)
    {
        // Java: the fix for the version above dataVersion applies
        if (fixer->fixes[i].version <= dataVersion)
            continue;
        if (fixer->fixes[i].version > currentVersion)
            break;
        data = fixer->fixes[i].fix(data, fixer->fixes[i].self);
        if (data == NULL)
            return NULL;
    }
    return data;
}

void LIBMATTI_DFUP_DataFixerUpper_Free(LIBMATTI_DFUP_DataFixerUpper *fixer)
{
    if (fixer == NULL)
        return;
    for (size_t i = 0; i < fixer->fixCount; i++)
        free(fixer->fixes[i].name);
    free(fixer->fixes);
    free(fixer);
}

// Java: DataFixUtils.updateNamedChoice / the rename helper the fixes use
LIBMATTI_DFUP_Dynamic *LIBMATTI_DFUP_DataFixUtils_UpdateNamed(LIBMATTI_DFUP_Dynamic *data, const char *oldName,
                                                              const char *newName)
{
    // Renames the top-level "Name"/"id" style entry when it matches oldName
    LIBMATTI_DFUP_Dynamic *name = LIBMATTI_DFUP_Dynamic_Get(data, "Name");
    if (name != NULL && name->type == LIBMATTI_DFUP_ValueType_STRING &&
        strcmp(name->stringValue, oldName) == 0)
    {
        return LIBMATTI_DFUP_Dynamic_Set(data, "Name", LIBMATTI_DFUP_Dynamic_String(newName));
    }

    LIBMATTI_DFUP_Dynamic *id = LIBMATTI_DFUP_Dynamic_Get(data, "id");
    if (id != NULL && id->type == LIBMATTI_DFUP_ValueType_STRING && strcmp(id->stringValue, oldName) == 0)
        return LIBMATTI_DFUP_Dynamic_Set(data, "id", LIBMATTI_DFUP_Dynamic_String(newName));
    return data;
}
