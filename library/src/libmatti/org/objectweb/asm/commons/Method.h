// Port of org.objectweb.asm.commons.Method.

#ifndef MATTICRAFT_ASM_COMMONS_METHOD_H
#define MATTICRAFT_ASM_COMMONS_METHOD_H

#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: public static final String CONSTRUCTOR_NAME = "<init>"
#define LIBMATTI_ASM_COMMONS_METHOD_CONSTRUCTOR_NAME "<init>"

// Java: public final class Method
typedef struct LIBMATTI_ASM_COMMONS_Method
{
    // Java: private final String name
    char *name;
    // Java: private final String descriptor
    char *descriptor;
} LIBMATTI_ASM_COMMONS_Method;

// Java: public Method(String name, String descriptor)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_New(const char *name, const char *descriptor);
// Java: public Method(String name, Type returnType, Type... argumentTypes)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_NewWithTypes(const char *name,
                                                                     const LIBMATTI_ASM_Type *returnType,
                                                                     LIBMATTI_ASM_Type **argumentTypes,
                                                                     size_t argumentCount);
// Java: public static Method getMethod(Method method)
LIBMATTI_ASM_COMMONS_Method *LIBMATTI_ASM_COMMONS_Method_GetMethod(const LIBMATTI_ASM_COMMONS_Method *method);
void LIBMATTI_ASM_COMMONS_Method_Free(LIBMATTI_ASM_COMMONS_Method *method);

// Java: public String getName()
const char *LIBMATTI_ASM_COMMONS_Method_GetName(const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public String getDescriptor()
const char *LIBMATTI_ASM_COMMONS_Method_GetDescriptor(const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public Type getReturnType()
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_Method_GetReturnType(const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public Type[] getArgumentTypes()
LIBMATTI_ASM_Type **LIBMATTI_ASM_COMMONS_Method_GetArgumentTypes(const LIBMATTI_ASM_COMMONS_Method *method,
                                                                size_t *count);

// Java: @Override public String toString() - name + descriptor
char *LIBMATTI_ASM_COMMONS_Method_ToString(const LIBMATTI_ASM_COMMONS_Method *method);
// Java: @Override public boolean equals(Object other)
int LIBMATTI_ASM_COMMONS_Method_Equals(const LIBMATTI_ASM_COMMONS_Method *method,
                                       const LIBMATTI_ASM_COMMONS_Method *other);
// Java: @Override public int hashCode()
int LIBMATTI_ASM_COMMONS_Method_HashCode(const LIBMATTI_ASM_COMMONS_Method *method);

#endif //MATTICRAFT_ASM_COMMONS_METHOD_H
