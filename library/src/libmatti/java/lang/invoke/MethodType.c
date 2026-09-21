#include "libmatti/java/lang/invoke/MethodType.h"

#include <stdlib.h>

LIBMATTI_JLI_MethodType *LIBMATTI_JLI_MethodType_MethodType(LIBMATTI_JL_Class *returnType,
                                                            LIBMATTI_JL_Class **parameterTypes, size_t parameterCount)
{
    LIBMATTI_JLI_MethodType *type = calloc(1, sizeof(LIBMATTI_JLI_MethodType));
    type->returnType = returnType;
    type->parameterTypes = parameterTypes;
    type->parameterCount = parameterCount;
    return type;
}

void LIBMATTI_JLI_MethodType_Free(LIBMATTI_JLI_MethodType *type)
{
    if (type == NULL) return;
    free(type->parameterTypes);
    free(type);
}
