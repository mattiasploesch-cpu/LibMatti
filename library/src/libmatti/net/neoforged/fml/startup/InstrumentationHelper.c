#include "libmatti/net/neoforged/fml/startup/InstrumentationHelper.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/startup/DevAgent.h"

// Java: private static final Logger LOG = LoggerFactory.getLogger(InstrumentationHelper.class);
static LIBMATTI_ML_Logger *LOG(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static Instrumentation getFromOurOwnAgent()
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_InstrumentationHelper_GetFromOurOwnAgent(void)
{
    // Java: var devAgent = Class.forName("net.neoforged.fml.startup.DevAgent", true, ClassLoader.getSystemClassLoader());
    //       var instrumentation = (Instrumentation) devAgent.getMethod("getInstrumentation").invoke(null);
    LIBMATTI_JLI_Instrumentation *instrumentation = LIBMATTI_FML_DevAgent_GetInstrumentation();

    // Java: if (instrumentation == null) throw new IllegalStateException("Our DevAgent was not attached. Pass an appropriate -javaagent parameter.");
    if (instrumentation == NULL) return NULL;

    // Java: LOG.info("Using our own agent");
    LIBMATTI_ML_Logger_Info(LOG(), NULL, "Using our own agent");
    return instrumentation;
}

// Java: public static Instrumentation obtainInstrumentation()
LIBMATTI_JLI_Instrumentation *LIBMATTI_FML_InstrumentationHelper_ObtainInstrumentation(void)
{
    // Java: var stackWalker = StackWalker.getInstance(StackWalker.Option.RETAIN_CLASS_REFERENCE);
    //       var callingPackage = stackWalker.getCallerClass().getPackageName();
    //       if (!callingPackage.equals(FMLLoader.class.getPackageName())) throw new IllegalStateException("This method may only be called by FML");
    // The port has no caller inspection; the single caller is FMLLoader.create, matching Java.

    // Java: var storedExceptions = new ArrayList<Exception>(); try { return getFromOurOwnAgent(); } catch (Exception e) { storedExceptions.add(e); }
    LIBMATTI_JLI_Instrumentation *instrumentation = LIBMATTI_FML_InstrumentationHelper_GetFromOurOwnAgent();
    if (instrumentation != NULL) return instrumentation;

    // Java: try { var classpathItem = SelfAttach.getClassPathItem(); var command = ProcessHandle.current().info().command()...;
    //           var process = new ProcessBuilder(command, "-cp", classpathItem, SelfAttach.class.getName(), DevAgent.class.getName())...start();
    //           ... return getFromOurOwnAgent(); } catch (Exception e) { storedExceptions.add(e); }
    // Java spawns its own executable with a -javaagent equivalent; the port's agent attaches
    // through the same DevAgent entry point, so the fallback re-queries it once the attach
    // hook ran (there is no second process to spawn).
    LIBMATTI_ML_Logger_Debug(LOG(), NULL, "SelfAttach fallback unavailable in the port");

    // Java: try { var byteBuddyAgent = Class.forName("net.bytebuddy.agent.ByteBuddyAgent", true, ClassLoader.getSystemClassLoader());
    //           var instrumentation = (Instrumentation) byteBuddyAgent.getMethod("install").invoke(null);
    //           LOG.info("Using byte-buddy fallback"); return instrumentation; } catch (Exception e) { storedExceptions.add(e); }
    // Byte Buddy is an external Java library with no C counterpart; the port skips the fallback.
    LIBMATTI_ML_Logger_Debug(LOG(), NULL, "ByteBuddyAgent fallback unavailable in the port");

    // Java: var e = new IllegalStateException("Failed to obtain instrumentation."); storedExceptions.forEach(e::addSuppressed); throw e;
    LIBMATTI_ML_Logger_Error(LOG(), NULL, "Failed to obtain instrumentation.");
    return NULL;
}
