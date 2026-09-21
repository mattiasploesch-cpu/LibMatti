// Port of com.mojang.brigadier.arguments.*: ArgumentType and the built-in
// Bool/Integer/Long/Float/Double/String argument types.
//
// Java's generics collapse to an enum discriminator + a union result; parse
// reports CommandSyntaxException through the out-parameter like the StringReader.

#ifndef MATTICRAFT_BR_ARGUMENTS_ARGUMENTTYPE_H
#define MATTICRAFT_BR_ARGUMENTS_ARGUMENTTYPE_H

#include "libmatti/com/mojang/brigadier/StringReader.h"

// Java: the concrete ArgumentType implementations
typedef enum
{
    LIBMATTI_BR_ARG_BOOL = 0,
    LIBMATTI_BR_ARG_INTEGER,
    LIBMATTI_BR_ARG_LONG,
    LIBMATTI_BR_ARG_FLOAT,
    LIBMATTI_BR_ARG_DOUBLE,
    LIBMATTI_BR_ARG_STRING
} LIBMATTI_BR_ArgumentKind;

// Java: StringArgumentType.StringType
typedef enum
{
    LIBMATTI_BR_StringType_SINGLE_WORD = 0,
    LIBMATTI_BR_StringType_QUOTABLE_PHRASE,
    LIBMATTI_BR_StringType_GREEDY_PHRASE
} LIBMATTI_BR_StringType;

// Java: public interface ArgumentType<T> - the port's value union
typedef union
{
    int boolValue;
    int intValue;
    long long longValue;
    float floatValue;
    double doubleValue;
    char *stringValue; // owned by the ParsedArgument
} LIBMATTI_BR_ArgumentValue;

typedef struct LIBMATTI_BR_ArgumentType
{
    LIBMATTI_BR_ArgumentKind kind;
    // Java: the numeric bounds (unused for bool/string)
    int minimum;
    int maximum;
    long long minimumLong;
    long long maximumLong;
    double minimumDouble;
    double maximumDouble;
    // Java: StringArgumentType.type
    LIBMATTI_BR_StringType stringType;
} LIBMATTI_BR_ArgumentType;

// Java: BoolArgumentType.bool()
LIBMATTI_BR_ArgumentType LIBMATTI_BR_BoolArgumentType_Bool(void);
// Java: IntegerArgumentType.integer() / integer(min) / integer(min, max)
LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_Integer(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_IntegerMin(int min);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_IntegerArgumentType_IntegerMinMax(int min, int max);
// Java: LongArgumentType.longArg() / longArg(min) / longArg(min, max)
LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_Long(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_LongMin(long long min);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_LongArgumentType_LongMinMax(long long min, long long max);
// Java: FloatArgumentType.floatArg() / floatArg(min) / floatArg(min, max)
LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_Float(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_FloatMin(float min);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_FloatArgumentType_FloatMinMax(float min, float max);
// Java: DoubleArgumentType.doubleArg() / doubleArg(min) / doubleArg(min, max)
LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_Double(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_DoubleMin(double min);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_DoubleArgumentType_DoubleMinMax(double min, double max);
// Java: StringArgumentType.word() / string() / greedyString()
LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_Word(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_String(void);
LIBMATTI_BR_ArgumentType LIBMATTI_BR_StringArgumentType_GreedyString(void);

// Java: T parse(StringReader) - one entry point per kind; on failure *outException
// is set and the returned value is undefined
int LIBMATTI_BR_ArgumentType_ParseBool(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                       LIBMATTI_BR_CommandSyntaxException **outException);
int LIBMATTI_BR_ArgumentType_ParseInt(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                      LIBMATTI_BR_CommandSyntaxException **outException);
long long LIBMATTI_BR_ArgumentType_ParseLong(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                             LIBMATTI_BR_CommandSyntaxException **outException);
float LIBMATTI_BR_ArgumentType_ParseFloat(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                          LIBMATTI_BR_CommandSyntaxException **outException);
double LIBMATTI_BR_ArgumentType_ParseDouble(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                            LIBMATTI_BR_CommandSyntaxException **outException);
// The caller frees the string (the union owns nothing afterwards)
char *LIBMATTI_BR_ArgumentType_ParseString(const LIBMATTI_BR_ArgumentType *type, LIBMATTI_BR_StringReader *reader,
                                           LIBMATTI_BR_CommandSyntaxException **outException);

// Java: StringArgumentType.escapeIfRequired(String) - the caller frees
char *LIBMATTI_BR_StringArgumentType_EscapeIfRequired(const char *input);

#endif
