#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMINGTHROWABLEPATTERNCONVERTER_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMINGTHROWABLEPATTERNCONVERTER_H

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/org/apache/logging/log4j/core/LogEvent.h"
#include "libmatti/org/apache/logging/log4j/core/config/Configuration.h"
#include "libmatti/org/apache/logging/log4j/core/pattern/ThrowablePatternConverter.h"

#define LIBMATTI_ML_TransformingThrowablePatternConverter_SUFFIXFLAG \
    "\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83SUFFIXFLAG\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83\xE2\x98\x83"
#define LIBMATTI_ML_TransformingThrowablePatternConverter_CONVERTER_KEYS "tEx"

typedef struct LIBMATTI_ML_TransformingThrowablePatternConverter
{
    LIBMATTI_L4J_ThrowablePatternConverter base;
} LIBMATTI_ML_TransformingThrowablePatternConverter;

LIBMATTI_ML_TransformingThrowablePatternConverter *LIBMATTI_ML_TransformingThrowablePatternConverter_New(
    LIBMATTI_L4J_Configuration *config, const LIBMATTI_L4J_Options *options);
void LIBMATTI_ML_TransformingThrowablePatternConverter_Free(
    LIBMATTI_ML_TransformingThrowablePatternConverter *converter);

LIBMATTI_ML_TransformingThrowablePatternConverter *LIBMATTI_ML_TransformingThrowablePatternConverter_NewInstance(
    LIBMATTI_L4J_Configuration *config, const LIBMATTI_L4J_Options *options);

char *LIBMATTI_ML_TransformingThrowablePatternConverter_GenerateEnhancedStackTrace(
    const LIBMATTI_JL_Throwable *throwable);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMINGTHROWABLEPATTERNCONVERTER_H
