#include "libmatti/net/neoforged/fml/startup/DevAgent.h"
#include "libmatti/net/neoforged/fml/startup/Entrypoint.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/io/PrintWriter.h"
#include "libmatti/java/io/StringWriter.h"
#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/java/lang/invoke/MethodHandles.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

// Java: startup() and createMainMethodCallable() throw; the C port cannot, so the
// entrypoint's catch clause reads the thrown exception back here.
static LIBMATTI_JL_Throwable *lastException = NULL;

LIBMATTI_JL_Throwable *LIBMATTI_FML_Entrypoint_GetLastException(void)
{
    return lastException;
}

// Java: protected static StartupResult startup(...)
static LIBMATTI_JL_Throwable *fatal_startup_exception(const char *message, LIBMATTI_FML_StartupArgs *startupArgs,
                                                     LIBMATTI_JL_Throwable *cause)
{
    return &LIBMATTI_FML_FatalStartupException_NewWithCause(message, startupArgs, cause)->base;
}

// Java: String concatenation
static char *concat(const char *a, const char *b, const char *c)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_NewFromString(a);
    if (b != NULL) LIBMATTI_JL_StringBuilder_Append(builder, b);
    if (c != NULL) LIBMATTI_JL_StringBuilder_Append(builder, c);

    char *result = strdup(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return result;
}

// Java: private static String getArg(String[] args, String name, String defaultValue)
static const char *get_arg(int argc, char *argv[], const char *name, const char *defaultValue)
{
    char argName[256];
    snprintf(argName, sizeof(argName), "--%s", name);

    for (int i = 0; i + 1 < argc; i++)
    {
        if (strcmp(argName, argv[i]) == 0)
        {
            return argv[i + 1];
        }
    }

    return defaultValue;
}

// Java: private static Path getGameDir(String[] args)
static char *get_game_dir(int argc, char *argv[])
{
    const char *gameDir = get_arg(argc, argv, "gameDir", "");

    // Java: new File(...).getAbsoluteFile()
    char *absolute = realpath(gameDir[0] != '\0' ? gameDir : ".", NULL);

    struct stat status;
    if (absolute == NULL || stat(absolute, &status) != 0 || !S_ISDIR(status.st_mode))
    {
        // Java: throw new RuntimeException("The game directory passed on the command-line is not a directory: " + gameDir)
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "The game directory passed on the command-line is not a directory: {}",
                                 gameDir);
        free(absolute);
        return NULL;
    }

    return absolute;
}

// Java: private static List<File> listClasspathEntries()
static void list_classpath_entries(char ***entries, size_t *count)
{
    *entries = NULL;
    *count = 0;

    // Java: System.getProperty("java.class.path").split(File.pathSeparator)
    const char *classPath = getenv("java.class.path");
    if (classPath == NULL) return;

    char *copy = strdup(classPath);
    char *saveptr = NULL;

    for (char *entry = strtok_r(copy, ":", &saveptr); entry != NULL; entry = strtok_r(NULL, ":", &saveptr))
    {
        *entries = realloc(*entries, sizeof(**entries) * (*count + 1));
        (*entries)[*count] = strdup(entry);
        (*count)++;
    }

    free(copy);
}

// Java: private static boolean hasCustomLoggingConfiguration()
static int has_custom_logging_configuration(void)
{
    return getenv("log4j2.configurationFile") != NULL
           || getenv("log4j.configurationFile") != NULL
           || getenv("LOG4J_CONFIGURATION_FILE") != NULL;
}

// Java: static void overwriteLoggingConfiguration()
static void overwrite_logging_configuration(void)
{
    // Java: if (System.getProperty("log4j2.disable.jmx") == null && System.getenv("LOG4J_DISABLE_JMX") == null)
    //           System.setProperty("log4j2.disable.jmx", "true");
    if (getenv("log4j2.disable.jmx") == NULL && getenv("LOG4J_DISABLE_JMX") == NULL)
        setenv("log4j2.disable.jmx", "true", 1);

    // Java: var loggingConfigUrl = Entrypoint.class.getResource("log4j2.xml");
    //       if (loggingConfigUrl == null) return;
    //       var configSource = ConfigurationSource.fromUri(loggingConfigUri);
    //       Configurator.reconfigure(ConfigurationFactory.getInstance().getConfiguration(LoggerContext.getContext(), configSource));
    //       StatusLogger.getLogger().debug("Reconfiguring logging with configuration from {}", loggingConfigUri);
    // The port's LogManager reads its level configuration directly; a missing log4j2.xml
    // resource leaves the defaults in place exactly like Java's early return.
    LIBMATTI_JL_ClassLoader *classLoader =
        LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread());
    LIBMATTI_JN_URL *loggingConfigUrl = LIBMATTI_JL_ClassLoader_GetResource(classLoader, "log4j2.xml");
    if (loggingConfigUrl != NULL)
    {
        LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "Reconfiguring logging with configuration from {}", "log4j2.xml");
    }
}

// Java: protected Entrypoint() {}
LIBMATTI_FML_Entrypoint *LIBMATTI_FML_Entrypoint_New(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_Entrypoint));
}

void LIBMATTI_FML_Entrypoint_Free(LIBMATTI_FML_Entrypoint *entrypoint)
{
    free(entrypoint);
}

// Java: protected static StartupResult startup(String[] args, boolean headless, Dist dist, boolean cleanDist)
LIBMATTI_FML_Entrypoint_StartupResult *LIBMATTI_FML_Entrypoint_Startup(int argc, char *argv[], int headless,
                                                                      LIBMATTI_DIST_Dist dist, int cleanDist)
{
    // Java: long startupUptime = ManagementFactory.getRuntimeMXBean().getUptime();
    // the port takes the process start time as the "VM start" (getUptime = now - start)
    static long long startupUptimeMillis = -1;
    if (startupUptimeMillis < 0)
    {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        startupUptimeMillis = (long long) now.tv_sec * 1000 + now.tv_nsec / 1000000;
    }
    struct timespec uptimeNow;
    clock_gettime(CLOCK_MONOTONIC, &uptimeNow);
    long long uptime = (long long) uptimeNow.tv_sec * 1000 + uptimeNow.tv_nsec / 1000000 - startupUptimeMillis;

    char startupUptime[32];
    snprintf(startupUptime, sizeof(startupUptime), "%lld", uptime);

    // Java: if (!hasCustomLoggingConfiguration()) { overwriteLoggingConfiguration(); }
    if (!has_custom_logging_configuration())
    {
        overwrite_logging_configuration();
    }

    // Java: var logger = LoggerFactory.getLogger(Entrypoint.class);
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();

    // Java: logger.info("JVM Uptime at startup: {}ms", startupUptime);
    LIBMATTI_ML_Logger_Info(logger, NULL, "JVM Uptime at startup: {}ms", startupUptime);

    LIBMATTI_FML_StartupArgs *startupArgs = calloc(1, sizeof(LIBMATTI_FML_StartupArgs));

    // Java: getGameDir(args)
    startupArgs->gameDirectory = get_game_dir(argc, argv);
    if (startupArgs->gameDirectory == NULL)
    {
        LIBMATTI_FML_StartupArgs_Free(startupArgs);
        return NULL;
    }

    startupArgs->headless = headless;
    startupArgs->dist = dist;
    startupArgs->cleanDist = cleanDist;
    startupArgs->argc = argc;
    startupArgs->programArgs = argv;

    // Java: new HashSet<>()
    startupArgs->claimedFileCount = 0;

    // Java: listClasspathEntries()
    list_classpath_entries(&startupArgs->unclaimedClassPathEntries, &startupArgs->unclaimedClassPathEntryCount);

    // Java: Thread.currentThread().getContextClassLoader()
    startupArgs->parentClassLoader =
        LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread());

    // Java: return new StartupResult(FMLLoader.create(DevAgent.getInstrumentation(), startupArgs), startupArgs);
    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_CreateWithInstrumentation(
        LIBMATTI_FML_DevAgent_GetInstrumentation(), startupArgs);

    // Java: create(...) threw -> the catch clause below runs
    if (loader == NULL)
    {
        // Java: catch (Exception e) {
        //           var sw = new StringWriter();
        //           e.printStackTrace(new PrintWriter(sw));
        //           logger.error("Failed to start FML: {}", sw);
        //           throw new FatalStartupException("Failed to start FML: " + e, startupArgs, e);
        //       }
        LIBMATTI_JL_Throwable *e = LIBMATTI_FML_FMLLoader_GetLastException();
        if (e == NULL)
            e = LIBMATTI_JL_Throwable_NewNamed("java.lang.RuntimeException", "FMLLoader.create failed");

        // Java: var sw = new StringWriter();
        LIBMATTI_JI_StringWriter *sw = LIBMATTI_JI_StringWriter_New();
        // Java: e.printStackTrace(new PrintWriter(sw));
        LIBMATTI_JI_PrintWriter *pw = LIBMATTI_JI_PrintWriter_New(&sw->base);
        LIBMATTI_JL_Throwable_PrintStackTraceTo(e, pw);
        LIBMATTI_JI_PrintWriter_Flush(pw);

        // Java: logger.error("Failed to start FML: {}", sw);
        LIBMATTI_ML_Logger_Error(logger, NULL, "Failed to start FML: {}", LIBMATTI_JI_StringWriter_ToString(sw));

        // Java: throw new FatalStartupException("Failed to start FML: " + e, startupArgs, e);
        LIBMATTI_JL_StringBuilder *message = LIBMATTI_JL_StringBuilder_NewFromString("Failed to start FML: ");
        LIBMATTI_JL_StringBuilder_Append(message, LIBMATTI_JL_Throwable_GetName(e));
        LIBMATTI_JL_StringBuilder_Append(message, ": ");
        LIBMATTI_JL_StringBuilder_Append(message, LIBMATTI_JL_Throwable_GetMessage(e));

        lastException = fatal_startup_exception(LIBMATTI_JL_StringBuilder_ToString(message), startupArgs, e);

        LIBMATTI_JL_StringBuilder_Free(message);
        LIBMATTI_JI_PrintWriter_Free(pw);
        LIBMATTI_JI_StringWriter_Free(sw);
        return NULL;
    }

    LIBMATTI_FML_Entrypoint_StartupResult *startupResult = calloc(1, sizeof(*startupResult));
    startupResult->loader = loader;
    startupResult->startupArgs = startupArgs;
    return startupResult;
}

// Java: protected static MethodHandle createMainMethodCallable(StartupResult startupResult, String mainClassName)
LIBMATTI_JLI_MethodHandle *LIBMATTI_FML_Entrypoint_CreateMainMethodCallable(
    LIBMATTI_FML_Entrypoint_StartupResult *startupResult, const char *mainClassName)
{
    // Java: var currentClassLoader = startupResult.loader.getCurrentClassLoader();
    LIBMATTI_JL_ClassLoader *currentClassLoader =
        LIBMATTI_FML_FMLLoader_GetCurrentClassLoader(startupResult->loader);

    // Java: var mainClass = Class.forName(mainClassName, true, currentClassLoader);
    LIBMATTI_JL_Class *mainClass = LIBMATTI_JL_Class_ForName(mainClassName, 1, currentClassLoader);
    if (mainClass == NULL)
    {
        // Java: catch (ClassNotFoundException e) {
        //           throw new FatalStartupException("Missing main class " + mainClassName + " on the classpath.", ...); }
        char *message = concat("Missing main class ", mainClassName, " on the classpath.");
        lastException = fatal_startup_exception(message, startupResult->startupArgs, NULL);
        free(message);
        return NULL;
    }

    // Java: if (mainClass.getClassLoader() != currentClassLoader) {
    //           throw new FatalStartupException("Missing main class " + mainClassName
    //                   + " from the game content loader (but available on " + mainClass.getClassLoader() + ").", ...); }
    if (LIBMATTI_JL_Class_GetClassLoader(mainClass) != currentClassLoader)
    {
        char *message = concat("Missing main class ", mainClassName, " from the game content loader.");
        lastException = fatal_startup_exception(message, startupResult->startupArgs, NULL);
        free(message);
        return NULL;
    }

    // Java: var lookup = MethodHandles.publicLookup();
    LIBMATTI_JLI_MethodHandles_Lookup *lookup = LIBMATTI_JLI_MethodHandles_PublicLookup();

    // Java: var methodType = MethodType.methodType(void.class, String[].class);
    LIBMATTI_JL_Class **parameterTypes = malloc(sizeof(*parameterTypes));
    parameterTypes[0] = LIBMATTI_JL_Class_StringArrayType();
    LIBMATTI_JLI_MethodType *methodType = LIBMATTI_JLI_MethodType_MethodType(LIBMATTI_JL_Class_VoidType(),
                                                                            parameterTypes, 1);

    // Java: return lookup.findStatic(mainClass, "main", methodType);
    LIBMATTI_JLI_MethodHandle *handle =
        LIBMATTI_JLI_MethodHandles_Lookup_FindStatic(lookup, mainClass, "main", methodType);
    if (handle == NULL)
    {
        // Java: catch (NoSuchMethodException e) {
        //           throw new FatalStartupException(mainClassName + " is missing a static 'main' method.", ...); }
        LIBMATTI_JLI_MethodType_Free(methodType);

        char *message = concat(mainClassName, " is missing a static 'main' method.", NULL);
        lastException = fatal_startup_exception(message, startupResult->startupArgs, NULL);
        free(message);
        return NULL;
    }

    return handle;
}

// Java: protected static @Nullable Thread findThread(String threadName)
void *LIBMATTI_FML_Entrypoint_FindThread(const char *threadName)
{
    // Java: Thread serverThread = null;
    //       for (var thread : Thread.getAllStackTraces().keySet()) {
    //           if (threadName.equals(thread.getName())) { if (serverThread == null || thread.threadId() <= serverThread.threadId()) serverThread = thread; } }
    //       return serverThread;
    size_t threadCount = 0;
    LIBMATTI_JL_Thread **threads = LIBMATTI_JL_Thread_GetAllThreads(&threadCount);
    LIBMATTI_JL_Thread *serverThread = NULL;
    for (size_t i = 0; i < threadCount; i++)
    {
        if (strcmp(LIBMATTI_JL_Thread_GetName(threads[i]), threadName) != 0) continue;
        if (serverThread == NULL ||
            LIBMATTI_JL_Thread_ThreadId(threads[i]) <= LIBMATTI_JL_Thread_ThreadId(serverThread))
        {
            serverThread = threads[i];
        }
    }
    free(threads);
    return serverThread;
}

// Java: the pending FatalStartupException references the StartupArgs record, so the GC keeps them alive
// as long as the exception is reachable. In C the pending exception owns them, so close() releases them
// only when nothing references them any more.
static int pending_exception_holds(const LIBMATTI_FML_StartupArgs *startupArgs)
{
    if (lastException == NULL) return 0;
    if (strcmp(LIBMATTI_JL_Throwable_GetName(lastException),
               "net.neoforged.fml.startup.FatalStartupException") != 0)
        return 0;

    return LIBMATTI_FML_FatalStartupException_GetStartupArgs(
               (const LIBMATTI_FML_FatalStartupException *) lastException) == startupArgs;
}

// Java: StartupResult.close() { loader.close(); }
void LIBMATTI_FML_Entrypoint_StartupResult_Close(LIBMATTI_FML_Entrypoint_StartupResult *startupResult)
{
    if (startupResult == NULL) return;

    // Java: loader.close();
    LIBMATTI_FML_FMLLoader_Close(startupResult->loader);
    LIBMATTI_FML_FMLLoader_Free(startupResult->loader);

    if (!pending_exception_holds(startupResult->startupArgs))
        LIBMATTI_FML_StartupArgs_Free(startupResult->startupArgs);
    free(startupResult);
}
