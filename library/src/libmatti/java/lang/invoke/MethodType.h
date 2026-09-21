#ifndef MATTICRAFT_JAVA_LANG_INVOKE_METHODTYPE_H
#define MATTICRAFT_JAVA_LANG_INVOKE_METHODTYPE_H

#include "libmatti/java/lang/Class.h"

#include <stddef.h>

// Java: public final class MethodType
typedef struct
{
    LIBMATTI_JL_Class *returnType;
    LIBMATTI_JL_Class **parameterTypes;
    size_t parameterCount;
} LIBMATTI_JLI_MethodType;

// Java: public static MethodType methodType(Class<?> rtype, Class<?>... ptypes)
LIBMATTI_JLI_MethodType *LIBMATTI_JLI_MethodType_MethodType(LIBMATTI_JL_Class *returnType,
                                                            LIBMATTI_JL_Class **parameterTypes, size_t parameterCount);
void LIBMATTI_JLI_MethodType_Free(LIBMATTI_JLI_MethodType *type);

#endif //MATTICRAFT_JAVA_LANG_INVOKE_METHODTYPE_H
