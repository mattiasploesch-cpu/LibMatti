// Port of net.neoforged.accesstransformer.parser.TargetType and
// net.neoforged.accesstransformer.parser.Transformation.
// See vendor/AccessTransformer/parser.

#ifndef MATTICRAFT_AT_TRANSFORMATION_H
#define MATTICRAFT_AT_TRANSFORMATION_H

#include <stddef.h>

// Java: public enum TargetType { FIELD, METHOD, CLASS }
typedef enum
{
    LIBMATTI_AT_TargetType_FIELD,
    LIBMATTI_AT_TargetType_METHOD,
    LIBMATTI_AT_TargetType_CLASS
} LIBMATTI_AT_TargetType;

// Java: public enum Modifier { PUBLIC, PRIVATE, PROTECTED, DEFAULT }
typedef enum
{
    LIBMATTI_AT_Modifier_PUBLIC,
    LIBMATTI_AT_Modifier_PRIVATE,
    LIBMATTI_AT_Modifier_PROTECTED,
    LIBMATTI_AT_Modifier_DEFAULT
} LIBMATTI_AT_Modifier;

// Java: public enum FinalState { LEAVE, MAKEFINAL, REMOVEFINAL }
typedef enum
{
    LIBMATTI_AT_FinalState_LEAVE,
    LIBMATTI_AT_FinalState_MAKEFINAL,
    LIBMATTI_AT_FinalState_REMOVEFINAL
} LIBMATTI_AT_FinalState;

// Java: public record Transformation(Modifier modifier, FinalState finalState, String origin, int line)
typedef struct
{
    LIBMATTI_AT_Modifier modifier;
    LIBMATTI_AT_FinalState finalState;
    char *origin; // owned
    int line;
} LIBMATTI_AT_Transformation;

LIBMATTI_AT_Transformation *LIBMATTI_AT_Transformation_New(LIBMATTI_AT_Modifier modifier,
                                                           LIBMATTI_AT_FinalState finalState,
                                                           const char *origin, int line);
void LIBMATTI_AT_Transformation_Free(LIBMATTI_AT_Transformation *transformation);

#endif //MATTICRAFT_AT_TRANSFORMATION_H
