// Port of org.objectweb.asm.Type.

#include "Type.h"

#include "Opcodes.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ASM_Type *type_new(int sort, const char *descriptor)
{
    LIBMATTI_ASM_Type *type = calloc(1, sizeof(LIBMATTI_ASM_Type));
    if (type == NULL) return NULL;

    type->sort = sort;
    type->descriptor = strdup(descriptor);

    return type;
}

static int descriptor_sort(char c)
{
    switch (c)
    {
    case 'V': return LIBMATTI_ASM_TYPE_VOID;
    case 'Z': return LIBMATTI_ASM_TYPE_BOOLEAN;
    case 'C': return LIBMATTI_ASM_TYPE_CHAR;
    case 'B': return LIBMATTI_ASM_TYPE_BYTE;
    case 'S': return LIBMATTI_ASM_TYPE_SHORT;
    case 'I': return LIBMATTI_ASM_TYPE_INT;
    case 'F': return LIBMATTI_ASM_TYPE_FLOAT;
    case 'J': return LIBMATTI_ASM_TYPE_LONG;
    case 'D': return LIBMATTI_ASM_TYPE_DOUBLE;
    case '[': return LIBMATTI_ASM_TYPE_ARRAY;
    case 'L': return LIBMATTI_ASM_TYPE_OBJECT;
    case '(': return LIBMATTI_ASM_TYPE_METHOD;
    default: return LIBMATTI_ASM_TYPE_VOID;
    }
}

LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetObjectType(const char *internalName)
{
    size_t length = strlen(internalName);
    char *descriptor = malloc(length + 3);
    descriptor[0] = 'L';
    memcpy(descriptor + 1, internalName, length);
    descriptor[length + 1] = ';';
    descriptor[length + 2] = '\0';

    LIBMATTI_ASM_Type *type = type_new(LIBMATTI_ASM_TYPE_OBJECT, descriptor);
    free(descriptor);

    return type;
}

LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetMethodType(const char *methodDescriptor)
{
    return type_new(LIBMATTI_ASM_TYPE_METHOD, methodDescriptor);
}

LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetType(const char *typeDescriptor)
{
    return type_new(descriptor_sort(typeDescriptor[0]), typeDescriptor);
}

const LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_VOID(void)
{
    static LIBMATTI_ASM_Type *voidType = NULL;
    if (voidType == NULL) voidType = type_new(LIBMATTI_ASM_TYPE_VOID, "V");
    return voidType;
}

int LIBMATTI_ASM_Type_GetSort(const LIBMATTI_ASM_Type *type)
{
    return type->sort;
}

const char *LIBMATTI_ASM_Type_GetDescriptor(const LIBMATTI_ASM_Type *type)
{
    return type->descriptor;
}

char *LIBMATTI_ASM_Type_GetInternalName(const LIBMATTI_ASM_Type *type)
{
    const char *descriptor = type->descriptor;
    size_t length = strlen(descriptor);
    if (type->sort == LIBMATTI_ASM_TYPE_OBJECT) return strndup(descriptor + 1, length - 2);
    if (type->sort == LIBMATTI_ASM_TYPE_ARRAY) return strdup(descriptor);
    return strdup(descriptor);
}

char *LIBMATTI_ASM_Type_GetClassName(const LIBMATTI_ASM_Type *type)
{
    char *name = LIBMATTI_ASM_Type_GetInternalName(type);
    if (name == NULL) return NULL;

    if (type->sort == LIBMATTI_ASM_TYPE_OBJECT || type->sort == LIBMATTI_ASM_TYPE_ARRAY)
        for (char *c = name; *c != '\0'; c++) if (*c == '/') *c = '.';

    return name;
}

int LIBMATTI_ASM_Type_GetSize(const LIBMATTI_ASM_Type *type)
{
    return type->sort == LIBMATTI_ASM_TYPE_LONG || type->sort == LIBMATTI_ASM_TYPE_DOUBLE ? 2 : 1;
}

int LIBMATTI_ASM_Type_GetDimensions(const LIBMATTI_ASM_Type *type)
{
    int dimensions = 0;
    while (type->descriptor[dimensions] == '[') dimensions++;
    return dimensions;
}

LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetElementType(const LIBMATTI_ASM_Type *type)
{
    int dimensions = LIBMATTI_ASM_Type_GetDimensions(type);
    if (dimensions == 0) return LIBMATTI_ASM_Type_GetType(type->descriptor);
    return LIBMATTI_ASM_Type_GetType(type->descriptor + dimensions);
}

LIBMATTI_ASM_Type **LIBMATTI_ASM_Type_GetArgumentTypes(const LIBMATTI_ASM_Type *type, size_t *count)
{
    const char *cursor = type->descriptor + 1;

    LIBMATTI_ASM_Type **arguments = NULL;
    size_t argumentCount = 0;

    while (*cursor != ')')
    {
        const char *end = cursor;
        while (*end == '[') end++;
        if (*end == 'L')
        {
            end = strchr(end, ';');
        }

        char *descriptor = strndup(cursor, (size_t)(end - cursor) + 1);
        LIBMATTI_ASM_Type *argument = LIBMATTI_ASM_Type_GetType(descriptor);
        free(descriptor);

        arguments = realloc(arguments, sizeof(*arguments) * (argumentCount + 1));
        arguments[argumentCount++] = argument;

        cursor = end + 1;
    }

    *count = argumentCount;
    return arguments;
}

LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetReturnType(const LIBMATTI_ASM_Type *type)
{
    const char *cursor = strchr(type->descriptor, ')');
    return LIBMATTI_ASM_Type_GetType(cursor + 1);
}

int LIBMATTI_ASM_Type_GetOpcode(const LIBMATTI_ASM_Type *type, int opcode)
{
    if (opcode == LIBMATTI_ASM_IALOAD || opcode == LIBMATTI_ASM_IASTORE)
    {
        switch (type->sort)
        {
        case LIBMATTI_ASM_TYPE_BOOLEAN:
        case LIBMATTI_ASM_TYPE_BYTE: return opcode + (LIBMATTI_ASM_BALOAD - LIBMATTI_ASM_IALOAD);
        case LIBMATTI_ASM_TYPE_CHAR: return opcode + (LIBMATTI_ASM_CALOAD - LIBMATTI_ASM_IALOAD);
        case LIBMATTI_ASM_TYPE_SHORT: return opcode + (LIBMATTI_ASM_SALOAD - LIBMATTI_ASM_IALOAD);
        default: return opcode;
        }
    }

    switch (type->sort)
    {
    case LIBMATTI_ASM_TYPE_VOID: return opcode;
    case LIBMATTI_ASM_TYPE_BOOLEAN:
    case LIBMATTI_ASM_TYPE_CHAR:
    case LIBMATTI_ASM_TYPE_BYTE:
    case LIBMATTI_ASM_TYPE_SHORT:
    case LIBMATTI_ASM_TYPE_INT: return opcode;
    case LIBMATTI_ASM_TYPE_FLOAT: return opcode + (LIBMATTI_ASM_FALOAD - LIBMATTI_ASM_IALOAD);
    case LIBMATTI_ASM_TYPE_LONG: return opcode + (LIBMATTI_ASM_LALOAD - LIBMATTI_ASM_IALOAD);
    case LIBMATTI_ASM_TYPE_DOUBLE: return opcode + (LIBMATTI_ASM_DALOAD - LIBMATTI_ASM_IALOAD);
    case LIBMATTI_ASM_TYPE_ARRAY:
    case LIBMATTI_ASM_TYPE_OBJECT: return opcode + (LIBMATTI_ASM_AALOAD - LIBMATTI_ASM_IALOAD);
    default: return -1;
    }
}

// Java: @Override public boolean equals(Object o)
int LIBMATTI_ASM_Type_Equals(const LIBMATTI_ASM_Type *type, const LIBMATTI_ASM_Type *other)
{
    if (other == NULL) return 0;
    return type->sort == other->sort && strcmp(type->descriptor, other->descriptor) == 0;
}

// Java: public static String getMethodDescriptor(Type returnType, Type... argumentTypes)
char *LIBMATTI_ASM_Type_GetMethodDescriptor(const LIBMATTI_ASM_Type *returnType,
                                            LIBMATTI_ASM_Type **argumentTypes, size_t argumentCount)
{
    size_t length = 2; // '(' and ')'
    for (size_t i = 0; i < argumentCount; i++) length += strlen(argumentTypes[i]->descriptor);
    length += strlen(returnType != NULL ? returnType->descriptor : "V");

    char *descriptor = malloc(length + 1);
    size_t written = 0;
    descriptor[written++] = '(';
    for (size_t i = 0; i < argumentCount; i++)
    {
        size_t argumentLength = strlen(argumentTypes[i]->descriptor);
        memcpy(descriptor + written, argumentTypes[i]->descriptor, argumentLength);
        written += argumentLength;
    }
    descriptor[written++] = ')';

    const char *returnDescriptor = returnType != NULL ? returnType->descriptor : "V";
    size_t returnLength = strlen(returnDescriptor);
    memcpy(descriptor + written, returnDescriptor, returnLength);
    written += returnLength;
    descriptor[written] = '\0';

    return descriptor;
}

void LIBMATTI_ASM_Type_Free(LIBMATTI_ASM_Type *type)
{
    if (type == NULL) return;
    free(type->descriptor);
    free(type);
}
