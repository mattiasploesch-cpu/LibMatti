// Port of net.neoforged.fml.startup.InstrumentationHelper.

#ifndef MATTICRAFT_FML_INSTRUMENTATIONHELPER_H
#define MATTICRAFT_FML_INSTRUMENTATIONHELPER_H

#include "libmatti/java/lang/instrument/Instrumentation.h"

// Java: public final class InstrumentationHelper
typedef struct LIBMATTI_FML_InstrumentationHelper
{
    int unused;
} LIBMATTI_FML_InstrumentationHelper;

// Java: public static Instrumentation obtainInstrumentation()
// NULL = the Java method threw IllegalStateException("Failed to obtain instrumentation.")
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_InstrumentationHelper_ObtainInstrumentation(void);

// Java: private static Instrumentation getFromOurOwnAgent()
// NULL = the Java method threw (DevAgent not attached / no instrumentation)
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_InstrumentationHelper_GetFromOurOwnAgent(void);

#endif //MATTICRAFT_FML_INSTRUMENTATIONHELPER_H
