// Port of net.neoforged.fml.startup.DevAgent.

#ifndef MATTICRAFT_FML_DEVAGENT_H
#define MATTICRAFT_FML_DEVAGENT_H

#include "libmatti/java/lang/instrument/Instrumentation.h"

// Java: public final class DevAgent
typedef struct LIBMATTI_FML_DevAgent
{
    int unused;
} LIBMATTI_FML_DevAgent;

// Java: public static Instrumentation getInstrumentation()
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_DevAgent_GetInstrumentation(void);

// Java: public static void premain(String arguments, Instrumentation instrumentation)
void LIBMATTI_FML_DevAgent_Premain(const char *arguments, LIBMATTI_JLI_Instrumentation *instrumentation);
// Java: public static void agentmain(String arguments, Instrumentation instrumentation)
void LIBMATTI_FML_DevAgent_Agentmain(const char *arguments, LIBMATTI_JLI_Instrumentation *instrumentation);

#endif //MATTICRAFT_FML_DEVAGENT_H
