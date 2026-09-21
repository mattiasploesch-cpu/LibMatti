// Port of the ASM "Object" values (Integer/Float/Long/Double/String/Type/Label).

#include "libmatti/org/objectweb/asm/Object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfInt(int value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_INT;
    object.intValue = value;
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfFloat(float value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_FLOAT;
    object.intValue = (int)value;
    object.doubleValue = value;
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfLong(long long value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_LONG;
    object.longValue = value;
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfDouble(double value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_DOUBLE;
    object.doubleValue = value;
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfString(const char *value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_STRING;
    object.stringValue = strdup(value);
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfType(const LIBMATTI_ASM_Type *value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_TYPE;
    object.typeValue = LIBMATTI_ASM_Type_GetType(LIBMATTI_ASM_Type_GetDescriptor(value));
    return object;
}

LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfLabel(struct LIBMATTI_ASM_Label *value)
{
    LIBMATTI_ASM_Object object = {0};
    object.kind = LIBMATTI_ASM_OBJ_LABEL;
    object.labelValue = value;
    return object;
}

char *LIBMATTI_ASM_Object_ToString(const LIBMATTI_ASM_Object *object)
{
    char buffer[64];

    switch (object->kind)
    {
    case LIBMATTI_ASM_OBJ_INT:
        snprintf(buffer, sizeof(buffer), "%d", object->intValue);
        return strdup(buffer);
    case LIBMATTI_ASM_OBJ_FLOAT:
        snprintf(buffer, sizeof(buffer), "%f", (float)object->doubleValue);
        return strdup(buffer);
    case LIBMATTI_ASM_OBJ_LONG:
        snprintf(buffer, sizeof(buffer), "%lld", object->longValue);
        return strdup(buffer);
    case LIBMATTI_ASM_OBJ_DOUBLE:
        snprintf(buffer, sizeof(buffer), "%f", object->doubleValue);
        return strdup(buffer);
    case LIBMATTI_ASM_OBJ_STRING:
        return strdup(object->stringValue);
    case LIBMATTI_ASM_OBJ_TYPE:
        return strdup(LIBMATTI_ASM_Type_GetDescriptor(object->typeValue));
    case LIBMATTI_ASM_OBJ_LABEL:
        return strdup("L");
    default:
        return strdup("null");
    }
}

int LIBMATTI_ASM_Object_Equals(const LIBMATTI_ASM_Object *a, const LIBMATTI_ASM_Object *b)
{
    if (a->kind != b->kind) return 0;

    switch (a->kind)
    {
    case LIBMATTI_ASM_OBJ_NONE: return 1;
    case LIBMATTI_ASM_OBJ_INT: return a->intValue == b->intValue;
    case LIBMATTI_ASM_OBJ_FLOAT: return (float)a->doubleValue == (float)b->doubleValue;
    case LIBMATTI_ASM_OBJ_LONG: return a->longValue == b->longValue;
    case LIBMATTI_ASM_OBJ_DOUBLE: return a->doubleValue == b->doubleValue;
    case LIBMATTI_ASM_OBJ_STRING: return strcmp(a->stringValue, b->stringValue) == 0;
    case LIBMATTI_ASM_OBJ_TYPE:
        return strcmp(LIBMATTI_ASM_Type_GetDescriptor(a->typeValue),
                      LIBMATTI_ASM_Type_GetDescriptor(b->typeValue)) == 0;
    case LIBMATTI_ASM_OBJ_LABEL: return a->labelValue == b->labelValue;
    default: return 0;
    }
}

void LIBMATTI_ASM_Object_Free(LIBMATTI_ASM_Object *object)
{
    if (object->kind == LIBMATTI_ASM_OBJ_STRING) free(object->stringValue);
    if (object->kind == LIBMATTI_ASM_OBJ_TYPE) LIBMATTI_ASM_Type_Free(object->typeValue);
    object->kind = LIBMATTI_ASM_OBJ_NONE;
}
