#include "libmatti/net/neoforged/fml/startup/DevAgent.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

static LIBMATTI_JLI_Instrumentation *instrumentation = NULL;

// Java: public static Instrumentation getInstrumentation()
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_DevAgent_GetInstrumentation(void)
{
    // Java: var stackWalker = StackWalker.getInstance(StackWalker.Option.RETAIN_CLASS_REFERENCE);
    //       var callingPackage = stackWalker.getCallerClass().getPackageName();
    //       if (!callingPackage.equals(DevAgent.class.getPackageName())) throw new IllegalStateException("This method may only be called by FML");
    // The port has no caller inspection; every caller is inside the FML port like in Java.
    return instrumentation;
}

// Java: public static void premain(String arguments, Instrumentation instrumentation)
void LIBMATTI_FML_DevAgent_Premain(const char *arguments, LIBMATTI_JLI_Instrumentation *newInstrumentation)
{
    // Java: DevAgent.instrumentation = instrumentation;
    instrumentation = newInstrumentation;
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Agent attached with arguments {}", arguments);
}

// Java: public static void agentmain(String arguments, Instrumentation instrumentation)
void LIBMATTI_FML_DevAgent_Agentmain(const char *arguments, LIBMATTI_JLI_Instrumentation *newInstrumentation)
{
    // Java: DevAgent.instrumentation = instrumentation;
    instrumentation = newInstrumentation;
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Agent attached with arguments {}", arguments);
}
