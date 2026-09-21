// Port of org.objectweb.asm.ConstantDynamic/Handle/Type "Object" values as used
// by ASM for constant values (field ConstantValue, LDC constants, frame values).

#ifndef MATTICRAFT_ASM_OBJECT_H
#define MATTICRAFT_ASM_OBJECT_H

#include "libmatti/org/objectweb/asm/Type.h"

typedef enum
{
    LIBMATTI_ASM_OBJ_NONE = 0,
    LIBMATTI_ASM_OBJ_INT,
    LIBMATTI_ASM_OBJ_FLOAT,
    LIBMATTI_ASM_OBJ_LONG,
    LIBMATTI_ASM_OBJ_DOUBLE,
    LIBMATTI_ASM_OBJ_STRING,
    LIBMATTI_ASM_OBJ_TYPE,
    // frame-only: an uninitialized value created by NEW (Java: Label reference)
    LIBMATTI_ASM_OBJ_LABEL,
    LIBMATTI_ASM_OBJ_HANDLE,
    LIBMATTI_ASM_OBJ_CONSTANT_DYNAMIC
} LIBMATTI_ASM_ObjectKind;

struct LIBMATTI_ASM_Label;

typedef struct
{
    int kind;
    int intValue;
    long long longValue;
    double doubleValue;
    char *stringValue;
    LIBMATTI_ASM_Type *typeValue;
    struct LIBMATTI_ASM_Label *labelValue;
    // Java: Handle / ConstantDynamic - resolved lazily from the constant pool
    int poolIndex;
} LIBMATTI_ASM_Object;

// Java: Integer.valueOf / Float.valueOf / ...
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfInt(int value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfFloat(float value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfLong(long long value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfDouble(double value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfString(const char *value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfType(const LIBMATTI_ASM_Type *value);
LIBMATTI_ASM_Object LIBMATTI_ASM_Object_OfLabel(struct LIBMATTI_ASM_Label *value);

// Java: Object.toString / equals
char *LIBMATTI_ASM_Object_ToString(const LIBMATTI_ASM_Object *object);
int LIBMATTI_ASM_Object_Equals(const LIBMATTI_ASM_Object *a, const LIBMATTI_ASM_Object *b);
void LIBMATTI_ASM_Object_Free(LIBMATTI_ASM_Object *object);

#endif //MATTICRAFT_ASM_OBJECT_H
