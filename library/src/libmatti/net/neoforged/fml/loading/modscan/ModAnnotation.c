// Port of net.neoforged.fml.loading.modscan.ModAnnotation.

#include "libmatti/net/neoforged/fml/loading/modscan/ModAnnotation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append(char **buffer, size_t *length, const char *text)
{
    if (text == NULL) return;

    size_t textLength = strlen(text);
    *buffer = realloc(*buffer, *length + textLength + 1);
    memcpy(*buffer + *length, text, textLength);
    *length += textLength;
    (*buffer)[*length] = '\0';
}

// Java: AbstractMap.toString() -> "{key=value, key2=value2}"
static char *map_to_string(const LIBMATTI_FML_ModAnnotationValues *values)
{
    char *result = strdup("");
    size_t length = 0;

    append(&result, &length, "{");
    for (size_t i = 0; i < values->count; i++)
    {
        if (i > 0) append(&result, &length, ", ");

        append(&result, &length, values->names[i]);
        append(&result, &length, "=");

        char *rendered = LIBMATTI_FML_ModAnnotationValue_ToString(values->values[i]);
        append(&result, &length, rendered);
        free(rendered);
    }
    append(&result, &length, "}");

    return result;
}

// Java: a List<Object> value; the port's AnnotationData carries the elements comma
// separated, which is what getDepends() and getSides() read.
static char *list_to_string(const LIBMATTI_FML_ModAnnotationValues *values)
{
    char *result = strdup("");
    size_t length = 0;

    for (size_t i = 0; i < values->count; i++)
    {
        if (i > 0) append(&result, &length, ",");

        char *rendered = LIBMATTI_FML_ModAnnotationValue_ToString(values->values[i]);
        append(&result, &length, rendered);
        free(rendered);
    }

    return result;
}

// Java: the Object values of the Map are carried as Strings in the port
// (ModFileScanData.AnnotationData); caller frees.
char *LIBMATTI_FML_ModAnnotationValue_ToString(const LIBMATTI_FML_ModAnnotationValue *value)
{
    if (value == NULL) return NULL;

    char buffer[64];
    switch (value->kind)
    {
    case LIBMATTI_FML_ModAnnotationValue_STRING:
        return strdup(value->stringValue != NULL ? value->stringValue : "");
    case LIBMATTI_FML_ModAnnotationValue_INT:
        snprintf(buffer, sizeof(buffer), "%d", (int) value->longValue);
        return strdup(buffer);
    case LIBMATTI_FML_ModAnnotationValue_LONG:
        snprintf(buffer, sizeof(buffer), "%ld", value->longValue);
        return strdup(buffer);
    case LIBMATTI_FML_ModAnnotationValue_DOUBLE:
    case LIBMATTI_FML_ModAnnotationValue_FLOAT:
        snprintf(buffer, sizeof(buffer), "%g", value->doubleValue);
        return strdup(buffer);
    case LIBMATTI_FML_ModAnnotationValue_BOOLEAN:
        return strdup(value->longValue != 0 ? "true" : "false");
    case LIBMATTI_FML_ModAnnotationValue_TYPE:
    {
        // Java: the Type is kept as is; the port renders its class name
        const LIBMATTI_ASM_Type *type = LIBMATTI_ASM_Type_GetType(value->stringValue);
        char *name = LIBMATTI_ASM_Type_GetClassName(type);
        LIBMATTI_ASM_Type_Free((LIBMATTI_ASM_Type *) type);
        return name;
    }
    case LIBMATTI_FML_ModAnnotationValue_ENUM:
        return strdup(value->enumHolder->value);
    case LIBMATTI_FML_ModAnnotationValue_ANNOTATION:
        return map_to_string(value->container);
    case LIBMATTI_FML_ModAnnotationValue_LIST:
        return list_to_string(value->container);
    }

    return NULL;
}

static LIBMATTI_FML_ModAnnotationValue *value_new(LIBMATTI_FML_ModAnnotationValueKind kind)
{
    LIBMATTI_FML_ModAnnotationValue *value = calloc(1, sizeof(LIBMATTI_FML_ModAnnotationValue));
    value->kind = kind;
    return value;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfString(const char *value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_STRING);
    result->stringValue = value != NULL ? strdup(value) : NULL;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfInt(int value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_INT);
    result->longValue = value;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfLong(long value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_LONG);
    result->longValue = value;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfDouble(double value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_DOUBLE);
    result->doubleValue = value;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfFloat(float value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_FLOAT);
    result->doubleValue = value;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfBoolean(int value)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_BOOLEAN);
    result->longValue = value;
    return result;
}

LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfType(const char *descriptor)
{
    LIBMATTI_FML_ModAnnotationValue *result = value_new(LIBMATTI_FML_ModAnnotationValue_TYPE);
    result->stringValue = descriptor != NULL ? strdup(descriptor) : NULL;
    return result;
}

static LIBMATTI_FML_ModAnnotationValues *values_new(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_ModAnnotationValues));
}

// Java: Map.put(key, value) - the port replaces the value of an existing key
static void values_put(LIBMATTI_FML_ModAnnotationValues *values, const char *key,
                       LIBMATTI_FML_ModAnnotationValue *value)
{
    if (key != NULL)
        for (size_t i = 0; i < values->count; i++)
            if (values->names[i] != NULL && strcmp(values->names[i], key) == 0)
            {
                LIBMATTI_FML_ModAnnotationValue_Free(values->values[i]);
                values->values[i] = value;
                return;
            }

    values->names = realloc(values->names, sizeof(char *) * (values->count + 1));
    values->values = realloc(values->values, sizeof(void *) * (values->count + 1));
    values->names[values->count] = key != NULL ? strdup(key) : NULL;
    values->values[values->count] = value;
    values->count++;
}

// Java: the ModAnnotation owns its values Map and, until endArray(), its arrayList.
// A value of kind ANNOTATION points at the Map of its child ModAnnotation, which the
// list frees, so it is not freed here.
void LIBMATTI_FML_ModAnnotationValue_Free(LIBMATTI_FML_ModAnnotationValue *value)
{
    if (value == NULL) return;

    free(value->stringValue);
    if (value->enumHolder != NULL)
    {
        free(value->enumHolder->desc);
        free(value->enumHolder->value);
        free(value->enumHolder);
    }
    if (value->kind == LIBMATTI_FML_ModAnnotationValue_LIST)
        LIBMATTI_FML_ModAnnotationValues_Free(value->container);

    free(value);
}

void LIBMATTI_FML_ModAnnotationValues_Free(LIBMATTI_FML_ModAnnotationValues *values)
{
    if (values == NULL) return;

    for (size_t i = 0; i < values->count; i++)
    {
        free(values->names[i]);
        LIBMATTI_FML_ModAnnotationValue_Free(values->values[i]);
    }
    free(values->names);
    free(values->values);
    free(values);
}

void LIBMATTI_FML_ModAnnotationList_AddFirst(LIBMATTI_FML_ModAnnotationList *list,
                                             LIBMATTI_FML_ModAnnotation *annotation)
{
    list->items = realloc(list->items, sizeof(void *) * (list->count + 1));
    for (size_t i = list->count; i > 0; i--) list->items[i] = list->items[i - 1];
    list->items[0] = annotation;
    list->count++;
}

void LIBMATTI_FML_ModAnnotationList_AddLast(LIBMATTI_FML_ModAnnotationList *list,
                                            LIBMATTI_FML_ModAnnotation *annotation)
{
    list->items = realloc(list->items, sizeof(void *) * (list->count + 1));
    list->items[list->count++] = annotation;
}

LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotationList_RemoveFirst(LIBMATTI_FML_ModAnnotationList *list)
{
    if (list->count == 0) return NULL;

    LIBMATTI_FML_ModAnnotation *annotation = list->items[0];
    for (size_t i = 1; i < list->count; i++) list->items[i - 1] = list->items[i];
    list->count--;
    return annotation;
}

LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotationList_GetFirst(const LIBMATTI_FML_ModAnnotationList *list)
{
    return list->count > 0 ? list->items[0] : NULL;
}

void LIBMATTI_FML_ModAnnotationList_Free(LIBMATTI_FML_ModAnnotationList *list)
{
    for (size_t i = 0; i < list->count; i++) LIBMATTI_FML_ModAnnotation_Free(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_New(LIBMATTI_NEOFORGESPI_ElementType type,
                                                           LIBMATTI_ASM_Type *asmType, const char *member)
{
    LIBMATTI_FML_ModAnnotation *annotation = calloc(1, sizeof(LIBMATTI_FML_ModAnnotation));
    annotation->type = type;
    // Java keeps the Type reference, which every caller made for this annotation only,
    // so the annotation owns and frees it.
    annotation->asmType = (LIBMATTI_ASM_Type *) asmType;
    annotation->member = member != NULL ? strdup(member) : NULL;
    // Java: private final Map<String, Object> values = Maps.newHashMap();
    annotation->values = values_new();
    return annotation;
}

LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_NewChildAnnotation(LIBMATTI_ASM_Type *asmType,
                                                                         const LIBMATTI_FML_ModAnnotation *parent)
{
    return LIBMATTI_FML_ModAnnotation_New(parent->type, asmType, parent->member);
}

void LIBMATTI_FML_ModAnnotation_Free(LIBMATTI_FML_ModAnnotation *annotation)
{
    if (annotation == NULL) return;

    LIBMATTI_ASM_Type_Free(annotation->asmType);
    free(annotation->member);
    free(annotation->arrayName);
    LIBMATTI_FML_ModAnnotationValues_Free(annotation->values);
    LIBMATTI_FML_ModAnnotationValues_Free(annotation->arrayList);
    free(annotation);
}

LIBMATTI_NEOFORGESPI_ElementType LIBMATTI_FML_ModAnnotation_GetType(const LIBMATTI_FML_ModAnnotation *annotation)
{
    return annotation->type;
}

const LIBMATTI_ASM_Type *LIBMATTI_FML_ModAnnotation_GetASMType(const LIBMATTI_FML_ModAnnotation *annotation)
{
    return annotation->asmType;
}

const char *LIBMATTI_FML_ModAnnotation_GetMember(const LIBMATTI_FML_ModAnnotation *annotation)
{
    return annotation->member;
}

const LIBMATTI_FML_ModAnnotationValues *LIBMATTI_FML_ModAnnotation_GetValues(
    const LIBMATTI_FML_ModAnnotation *annotation)
{
    return annotation->values;
}

// Java: public void addArray(String name)
void LIBMATTI_FML_ModAnnotation_AddArray(LIBMATTI_FML_ModAnnotation *annotation, const char *name)
{
    annotation->arrayList = values_new();
    free(annotation->arrayName);
    annotation->arrayName = name != NULL ? strdup(name) : NULL;
}

// Java: public void addProperty(String key, Object value)
void LIBMATTI_FML_ModAnnotation_AddProperty(LIBMATTI_FML_ModAnnotation *annotation, const char *key,
                                            LIBMATTI_FML_ModAnnotationValue *value)
{
    if (annotation->arrayList != NULL)
        values_put(annotation->arrayList, NULL, value);
    else
        values_put(annotation->values, key, value);
}

// Java: public void addEnumProperty(String key, String enumName, String value)
void LIBMATTI_FML_ModAnnotation_AddEnumProperty(LIBMATTI_FML_ModAnnotation *annotation, const char *key,
                                                const char *enumName, const char *value)
{
    LIBMATTI_FML_ModAnnotationValue *property = value_new(LIBMATTI_FML_ModAnnotationValue_ENUM);
    property->enumHolder = calloc(1, sizeof(LIBMATTI_FML_ModAnnotation_EnumHolder));
    property->enumHolder->desc = enumName != NULL ? strdup(enumName) : NULL;
    property->enumHolder->value = value != NULL ? strdup(value) : NULL;

    LIBMATTI_FML_ModAnnotation_AddProperty(annotation, key, property);
}

// Java: public void endArray()
void LIBMATTI_FML_ModAnnotation_EndArray(LIBMATTI_FML_ModAnnotation *annotation)
{
    LIBMATTI_FML_ModAnnotationValue *property = value_new(LIBMATTI_FML_ModAnnotationValue_LIST);
    property->container = annotation->arrayList;

    values_put(annotation->values, annotation->arrayName, property);
    annotation->arrayList = NULL;
    free(annotation->arrayName);
    annotation->arrayName = NULL;
}

// Java: public ModAnnotation addChildAnnotation(String name, String desc)
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_AddChildAnnotation(LIBMATTI_FML_ModAnnotation *annotation,
                                                                         const char *name, const char *desc)
{
    LIBMATTI_FML_ModAnnotation *child =
        LIBMATTI_FML_ModAnnotation_NewChildAnnotation(LIBMATTI_ASM_Type_GetType(desc), annotation);

    // Java: addProperty(name, child.getValues()) - the parent's value points at the child's Map
    LIBMATTI_FML_ModAnnotationValue *property = value_new(LIBMATTI_FML_ModAnnotationValue_ANNOTATION);
    property->container = child->values;
    LIBMATTI_FML_ModAnnotation_AddProperty(annotation, name, property);

    return child;
}

// Java: static AnnotationData fromModAnnotation(Type clazz, ModAnnotation annotation)
LIBMATTI_NEOFORGESPI_AnnotationData LIBMATTI_FML_ModAnnotation_FromModAnnotation(
    const char *clazz, const LIBMATTI_FML_ModAnnotation *annotation)
{
    LIBMATTI_NEOFORGESPI_AnnotationData data = {0};
    // Java: the Type objects move into the record (annotation.asmType, clazz)
    data.annotationType = LIBMATTI_ASM_Type_GetObjectType(LIBMATTI_ASM_Type_GetInternalName(annotation->asmType));
    data.targetType = annotation->type;
    data.clazz = clazz != NULL ? LIBMATTI_ASM_Type_GetObjectType(clazz) : NULL;
    data.memberName = annotation->member != NULL ? strdup(annotation->member) : NULL;

    const LIBMATTI_FML_ModAnnotationValues *values = annotation->values;
    data.annotationDataCount = values->count;
    if (values->count == 0) return data;

    data.annotationDataKeys = calloc(values->count, sizeof(char *));
    data.annotationDataValues = calloc(values->count, sizeof(char *));
    for (size_t i = 0; i < values->count; i++)
    {
        data.annotationDataKeys[i] = values->names[i] != NULL ? strdup(values->names[i]) : NULL;
        data.annotationDataValues[i] = LIBMATTI_FML_ModAnnotationValue_ToString(values->values[i]);
    }

    return data;
}
