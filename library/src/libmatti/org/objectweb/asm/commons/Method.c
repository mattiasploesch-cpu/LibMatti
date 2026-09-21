// Port of org.objectweb.asm.commons.Method.

#include "libmatti/org/objectweb/asm/commons/Method.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public Method(String name, String descriptor)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_New(const char *name, const char *descriptor)
{
    LIBMATTI_ASM_COMMONS_Method *method = calloc(1, sizeof(LIBMATTI_ASM_COMMONS_Method));
    method->name = name != NULL ? strdup(name) : NULL;
    method->descriptor = descriptor != NULL ? strdup(descriptor) : NULL;
    return method;
}

// Java: public Method(String name, Type returnType, Type... argumentTypes)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_NewWithTypes(const char *name,
                                                                     const LIBMATTI_ASM_Type *returnType,
                                                                     LIBMATTI_ASM_Type **argumentTypes,
                                                                     size_t argumentCount)
{
    // Java: this(name, Type.getMethodDescriptor(returnType, argumentTypes));
    char *descriptor = LIBMATTI_ASM_Type_GetMethodDescriptor(returnType, argumentTypes, argumentCount);
    LIBMATTI_ASM_COMMONS_Method *method = LIBMATTI_ASM_COMMONS_Method_New(name, descriptor);
    free(descriptor);
    return method;
}

// Java: public static Method getMethod(Method method)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_GetMethod(const LIBMATTI_ASM_COMMONS_Method *method)
{
    return LIBMATTI_ASM_COMMONS_Method_New(method->name, method->descriptor);
}

void LIBMATTI_ASM_COMMONS_Method_Free(LIBMATTI_ASM_COMMONS_Method *method)
{
    if (method == NULL) return;
    free(method->name);
    free(method->descriptor);
    free(method);
}

// Java: public String getName()
const char *LIBMATTI_ASM_COMMONS_Method_GetName(const LIBMATTI_ASM_COMMONS_Method *method)
{
    return method->name;
}

// Java: public String getDescriptor()
const char *LIBMATTI_ASM_COMMONS_Method_GetDescriptor(const LIBMATTI_ASM_COMMONS_Method *method)
{
    return method->descriptor;
}

// Java: public Type getReturnType() { return Type.getReturnType(descriptor); }
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_Method_GetReturnType(const LIBMATTI_ASM_COMMONS_Method *method)
{
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetMethodType(method->descriptor);
    LIBMATTI_ASM_Type *returnType = LIBMATTI_ASM_Type_GetReturnType(methodType);
    LIBMATTI_ASM_Type_Free(methodType);
    return returnType;
}

// Java: public Type[] getArgumentTypes() { return Type.getArgumentTypes(descriptor); }
LIBMATTI_ASM_Type **LIBMATTI_ASM_COMMONS_Method_GetArgumentTypes(const LIBMATTI_ASM_COMMONS_Method *method,
                                                                size_t *count)
{
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetMethodType(method->descriptor);
    LIBMATTI_ASM_Type **argumentTypes = LIBMATTI_ASM_Type_GetArgumentTypes(methodType, count);
    LIBMATTI_ASM_Type_Free(methodType);
    return argumentTypes;
}

// Java: @Override public String toString()
char *LIBMATTI_ASM_COMMONS_Method_ToString(const LIBMATTI_ASM_COMMONS_Method *method)
{
    size_t length = strlen(method->name) + strlen(method->descriptor) + 1;
    char *text = malloc(length);
    snprintf(text, length, "%s%s", method->name, method->descriptor);
    return text;
}

// Java: @Override public boolean equals(Object other)
int LIBMATTI_ASM_COMMONS_Method_Equals(const LIBMATTI_ASM_COMMONS_Method *method,
                                       const LIBMATTI_ASM_COMMONS_Method *other)
{
    if (other == NULL) return 0;
    return strcmp(method->name, other->name) == 0 && strcmp(method->descriptor, other->descriptor) == 0;
}

// Java: @Override public int hashCode() { return name.hashCode() ^ descriptor.hashCode(); }
int LIBMATTI_ASM_COMMONS_Method_HashCode(const LIBMATTI_ASM_COMMONS_Method *method)
{
    int nameHash = 0;
    for (const char *c = method->name; *c != '\0'; c++) nameHash = nameHash * 31 + (unsigned char) *c;

    int descriptorHash = 0;
    for (const char *c = method->descriptor; *c != '\0'; c++)
        descriptorHash = descriptorHash * 31 + (unsigned char) *c;

    return nameHash ^ descriptorHash;
}
