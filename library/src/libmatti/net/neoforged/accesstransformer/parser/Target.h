// Port of net.neoforged.accesstransformer.parser.Target.
// Java: sealed interface Target permits ClassTarget, FieldTarget, MethodTarget,
// WildcardFieldTarget, WildcardMethodTarget. The C port keeps the kind + names
// in one struct.

#ifndef MATTICRAFT_AT_TARGET_H
#define MATTICRAFT_AT_TARGET_H

#include "libmatti/net/neoforged/accesstransformer/parser/Transformation.h"

#include <stddef.h>

typedef enum
{
    LIBMATTI_AT_TargetKind_CLASS,
    LIBMATTI_AT_TargetKind_FIELD,
    LIBMATTI_AT_TargetKind_METHOD,
    LIBMATTI_AT_TargetKind_WILDCARD_FIELD,
    LIBMATTI_AT_TargetKind_WILDCARD_METHOD
} LIBMATTI_AT_TargetKind;

// Java: record Target.TargetType + the member name/descriptor
typedef struct
{
    LIBMATTI_AT_TargetKind kind;
    char *className;   // internal name (slashes), owned
    char *memberName;  // field/method name or NULL for class targets, owned
    char *descriptor;  // method descriptor or NULL, owned
} LIBMATTI_AT_Target;

// Java: new Target.ClassTarget(className) - className in dot form, stored internal
LIBMATTI_AT_Target *LIBMATTI_AT_Target_Class(const char *className);
LIBMATTI_AT_Target *LIBMATTI_AT_Target_Field(const char *className, const char *fieldName);
LIBMATTI_AT_Target *LIBMATTI_AT_Target_Method(const char *className, const char *methodName, const char *descriptor);
LIBMATTI_AT_Target *LIBMATTI_AT_Target_WildcardField(const char *className);
LIBMATTI_AT_Target *LIBMATTI_AT_Target_WildcardMethod(const char *className);
void LIBMATTI_AT_Target_Free(LIBMATTI_AT_Target *target);

// Java: the map key the AccessTransformerList uses
// class targets: internal class name; fields: name; methods: name+descriptor
char *LIBMATTI_AT_Target_ToString(const LIBMATTI_AT_Target *target); // caller frees

#endif //MATTICRAFT_AT_TARGET_H
