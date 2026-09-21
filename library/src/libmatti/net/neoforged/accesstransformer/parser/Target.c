#include "libmatti/net/neoforged/accesstransformer/parser/Target.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the AT files use dot-separated class names; the internal form uses slashes
static char *to_internal_name(const char *className)
{
    char *internal = strdup(className);
    for (char *c = internal; *c != '\0'; c++)
        if (*c == '.')
            *c = '/';
    return internal;
}

static LIBMATTI_AT_Target *target_new(LIBMATTI_AT_TargetKind kind, const char *className)
{
    LIBMATTI_AT_Target *target = calloc(1, sizeof(LIBMATTI_AT_Target));
    target->kind = kind;
    target->className = to_internal_name(className);
    return target;
}

LIBMATTI_AT_Target *LIBMATTI_AT_Target_Class(const char *className)
{
    return target_new(LIBMATTI_AT_TargetKind_CLASS, className);
}

LIBMATTI_AT_Target *LIBMATTI_AT_Target_Field(const char *className, const char *fieldName)
{
    LIBMATTI_AT_Target *target = target_new(LIBMATTI_AT_TargetKind_FIELD, className);
    target->memberName = strdup(fieldName);
    return target;
}

LIBMATTI_AT_Target *LIBMATTI_AT_Target_Method(const char *className, const char *methodName, const char *descriptor)
{
    LIBMATTI_AT_Target *target = target_new(LIBMATTI_AT_TargetKind_METHOD, className);
    target->memberName = strdup(methodName);
    target->descriptor = strdup(descriptor);
    return target;
}

LIBMATTI_AT_Target *LIBMATTI_AT_Target_WildcardField(const char *className)
{
    return target_new(LIBMATTI_AT_TargetKind_WILDCARD_FIELD, className);
}

LIBMATTI_AT_Target *LIBMATTI_AT_Target_WildcardMethod(const char *className)
{
    return target_new(LIBMATTI_AT_TargetKind_WILDCARD_METHOD, className);
}

void LIBMATTI_AT_Target_Free(LIBMATTI_AT_Target *target)
{
    if (target == NULL)
        return;
    free(target->className);
    free(target->memberName);
    free(target->descriptor);
    free(target);
}

char *LIBMATTI_AT_Target_ToString(const LIBMATTI_AT_Target *target)
{
    size_t length = strlen(target->className) + 2;
    if (target->memberName != NULL)
        length += strlen(target->memberName);
    if (target->descriptor != NULL)
        length += strlen(target->descriptor);

    char *key = malloc(length);
    if (target->memberName == NULL)
        strcpy(key, target->className);
    else if (target->descriptor == NULL)
        strcpy(key, target->memberName);
    else
        sprintf(key, "%s%s", target->memberName, target->descriptor);
    return key;
}
