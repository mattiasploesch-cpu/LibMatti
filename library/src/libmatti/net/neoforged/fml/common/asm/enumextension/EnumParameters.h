// Port of net.neoforged.fml.common.asm.enumextension.EnumParameters.
// Java: sealed interface EnumParameters with the records Constant(List<Object> params),
// FieldReference(Type owner, String fieldName) and MethodReference(Type owner, String methodName).
// Java's List<Object> is untyped; the port tags each constant with the wrapper type it was parsed as.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPARAMETERS_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPARAMETERS_H

#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: the runtime class of the constants EnumPrototype keeps in List<Object>
typedef enum
{
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_NULL = 0,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_STRING,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_CHARACTER,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BYTE,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_SHORT,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_INTEGER,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_LONG,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_FLOAT,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_DOUBLE,
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BOOLEAN
} LIBMATTI_FML_EnumParameters_ConstantType;

// Java: one element of the Constant record's List<Object>
typedef struct
{
    LIBMATTI_FML_EnumParameters_ConstantType type;
    // Java: String
    char *stringValue;
    // Java: Character / Byte / Short / Integer / Boolean
    int intValue;
    // Java: Long
    long long longValue;
    // Java: Float
    float floatValue;
    // Java: Double
    double doubleValue;
} LIBMATTI_FML_EnumParameters_Constant;

// Java: the three records of the sealed interface
typedef enum
{
    LIBMATTI_FML_ENUMPARAMETERS_CONSTANT = 0,
    LIBMATTI_FML_ENUMPARAMETERS_FIELD_REFERENCE,
    LIBMATTI_FML_ENUMPARAMETERS_METHOD_REFERENCE
} LIBMATTI_FML_EnumParameters_Kind;

// Java: sealed interface EnumParameters
typedef struct LIBMATTI_FML_EnumParameters
{
    LIBMATTI_FML_EnumParameters_Kind kind;

    // Java: record Constant(List<Object> params)
    LIBMATTI_FML_EnumParameters_Constant *params;
    size_t paramCount;

    // Java: record FieldReference(Type owner, String fieldName)
    // Java: record MethodReference(Type owner, String methodName)
    LIBMATTI_ASM_Type *owner;
    char *memberName;
} LIBMATTI_FML_EnumParameters;

// Java: new EnumParameters.Constant(params)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewConstant(
    LIBMATTI_FML_EnumParameters_Constant *params, size_t paramCount);
// Java: new EnumParameters.FieldReference(owner, fieldName)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewFieldReference(const LIBMATTI_ASM_Type *owner,
                                                                          const char *fieldName);
// Java: new EnumParameters.MethodReference(owner, methodName)
LIBMATTI_FML_EnumParameters *LIBMATTI_FML_EnumParameters_NewMethodReference(const LIBMATTI_ASM_Type *owner,
                                                                           const char *methodName);
void LIBMATTI_FML_EnumParameters_Free(LIBMATTI_FML_EnumParameters *parameters);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPARAMETERS_H
