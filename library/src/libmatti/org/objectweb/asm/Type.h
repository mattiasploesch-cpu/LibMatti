// Port of org.objectweb.asm.Type.

#ifndef MATTICRAFT_ASM_TYPE_H
#define MATTICRAFT_ASM_TYPE_H

#include <stddef.h>

typedef enum
{
    LIBMATTI_ASM_TYPE_VOID = 0,
    LIBMATTI_ASM_TYPE_BOOLEAN = 1,
    LIBMATTI_ASM_TYPE_CHAR = 2,
    LIBMATTI_ASM_TYPE_BYTE = 3,
    LIBMATTI_ASM_TYPE_SHORT = 4,
    LIBMATTI_ASM_TYPE_INT = 5,
    LIBMATTI_ASM_TYPE_FLOAT = 6,
    LIBMATTI_ASM_TYPE_LONG = 7,
    LIBMATTI_ASM_TYPE_DOUBLE = 8,
    LIBMATTI_ASM_TYPE_ARRAY = 9,
    LIBMATTI_ASM_TYPE_OBJECT = 10,
    LIBMATTI_ASM_TYPE_METHOD = 11
} LIBMATTI_ASM_TypeSort;

// Java: public final class Type
typedef struct LIBMATTI_ASM_Type
{
    int sort;
    char *descriptor;
} LIBMATTI_ASM_Type;

// Java: static Type getObjectType(String internalName)
LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetObjectType(const char *internalName);
// Java: static Type getMethodType(String methodDescriptor)
LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetMethodType(const char *methodDescriptor);
// Java: static Type getType(String typeDescriptor)
LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetType(const char *typeDescriptor);
// Java: static final Type VOID_TYPE
const LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_VOID(void);

// Java: int getSort()
int LIBMATTI_ASM_Type_GetSort(const LIBMATTI_ASM_Type *type);
// Java: String getDescriptor() - internal string, do not free
const char *LIBMATTI_ASM_Type_GetDescriptor(const LIBMATTI_ASM_Type *type);
// Java: String getInternalName(); caller frees
char *LIBMATTI_ASM_Type_GetInternalName(const LIBMATTI_ASM_Type *type);
// Java: String getClassName(); caller frees
char *LIBMATTI_ASM_Type_GetClassName(const LIBMATTI_ASM_Type *type);
// Java: int getSize()
int LIBMATTI_ASM_Type_GetSize(const LIBMATTI_ASM_Type *type);
// Java: int getDimensions()
int LIBMATTI_ASM_Type_GetDimensions(const LIBMATTI_ASM_Type *type);
// Java: Type getElementType()
LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetElementType(const LIBMATTI_ASM_Type *type);
// Java: Type[] getArgumentTypes(); caller frees array and each element
LIBMATTI_ASM_Type **LIBMATTI_ASM_Type_GetArgumentTypes(const LIBMATTI_ASM_Type *type, size_t *count);
// Java: Type getReturnType()
LIBMATTI_ASM_Type *LIBMATTI_ASM_Type_GetReturnType(const LIBMATTI_ASM_Type *type);
// Java: int getOpcode(int opcode)
int LIBMATTI_ASM_Type_GetOpcode(const LIBMATTI_ASM_Type *type, int opcode);
// Java: static String getMethodDescriptor(Type returnType, Type... argumentTypes)
char *LIBMATTI_ASM_Type_GetMethodDescriptor(const LIBMATTI_ASM_Type *returnType,
                                            LIBMATTI_ASM_Type **argumentTypes, size_t argumentCount);
// Java: @Override public boolean equals(Object o) - the descriptor comparison
int LIBMATTI_ASM_Type_Equals(const LIBMATTI_ASM_Type *type, const LIBMATTI_ASM_Type *other);

void LIBMATTI_ASM_Type_Free(LIBMATTI_ASM_Type *type);

#endif //MATTICRAFT_ASM_TYPE_H
