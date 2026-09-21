// Port of net.neoforged.fml.common.asm.enumextension.EnumParameters.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumParameters.h"

#include <stdlib.h>
#include <string.h>

// Java: record Constant(List<Object> params)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewConstant(
    LIBMATTI_FML_EnumParameters_Constant *params, size_t paramCount)
{
    LIBMATTI_FML_EnumParameters *parameters = calloc(1, sizeof(LIBMATTI_FML_EnumParameters));
    parameters->kind = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT;
    // Java: records are immutable; the port takes the parsed array over
    parameters->params = params;
    parameters->paramCount = paramCount;
    return parameters;
}

// Java: record FieldReference(Type owner, String fieldName)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewFieldReference(const LIBMATTI_ASM_Type *owner,
                                                                          const char *fieldName)
{
    LIBMATTI_FML_EnumParameters *parameters = calloc(1, sizeof(LIBMATTI_FML_EnumParameters));
    parameters->kind = LIBMATTI_FML_ENUMPARAMETERS_FIELD_REFERENCE;
    parameters->owner = LIBMATTI_ASM_Type_GetType(LIBMATTI_ASM_Type_GetDescriptor(owner));
    parameters->memberName = fieldName != NULL ? strdup(fieldName) : NULL;
    return parameters;
}

// Java: record MethodReference(Type owner, String methodName)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewMethodReference(const LIBMATTI_ASM_Type *owner,
                                                                           const char *methodName)
{
    LIBMATTI_FML_EnumParameters *parameters = calloc(1, sizeof(LIBMATTI_FML_EnumParameters));
    parameters->kind = LIBMATTI_FML_ENUMPARAMETERS_METHOD_REFERENCE;
    parameters->owner = LIBMATTI_ASM_Type_GetType(LIBMATTI_ASM_Type_GetDescriptor(owner));
    parameters->memberName = methodName != NULL ? strdup(methodName) : NULL;
    return parameters;
}

void LIBMATTI_FML_EnumParameters_Free(LIBMATTI_FML_EnumParameters *parameters)
{
    if (parameters == NULL) return;

    for (size_t i = 0; i < parameters->paramCount; i++)
        free(parameters->params[i].stringValue);
    free(parameters->params);

    LIBMATTI_ASM_Type_Free(parameters->owner);
    free(parameters->memberName);
    free(parameters);
}
