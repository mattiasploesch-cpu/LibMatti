#ifndef MATTICRAFT_FML_ENTRYPOINT_H
#define MATTICRAFT_FML_ENTRYPOINT_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/java/lang/invoke/MethodHandle.h"
#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/startup/FatalStartupException.h"
#include "libmatti/net/neoforged/fml/startup/StartupArgs.h"

#include <stddef.h>

// Java: public abstract class Entrypoint (no instance state, only static methods)
typedef struct LIBMATTI_FML_Entrypoint
{
    int unused;
} LIBMATTI_FML_Entrypoint;

// Java: protected record StartupResult(FMLLoader loader, StartupArgs startupArgs) implements AutoCloseable
typedef struct
{
    LIBMATTI_FML_FMLLoader *loader;
    LIBMATTI_FML_StartupArgs *startupArgs;
} LIBMATTI_FML_Entrypoint_StartupResult;

// Java: protected Entrypoint()
LIBMATTI_FML_Entrypoint *LIBMATTI_FML_Entrypoint_New(void);
void LIBMATTI_FML_Entrypoint_Free(LIBMATTI_FML_Entrypoint *entrypoint);

// Java: protected static StartupResult startup(String[] args, boolean headless, Dist dist, boolean cleanDist)
// NULL = the Java method threw a FatalStartupException (handled by the entrypoint's catch clause)
LIBMATTI_FML_Entrypoint_StartupResult *LIBMATTI_FML_Entrypoint_Startup(int argc, char *argv[], int headless,
                                                                      LIBMATTI_DIST_Dist dist, int cleanDist);

// Java: protected static MethodHandle createMainMethodCallable(StartupResult startupResult, String mainClassName)
// NULL = the Java method threw a FatalStartupException (read back with GetLastException)
LIBMATTI_JLI_MethodHandle *LIBMATTI_FML_Entrypoint_CreateMainMethodCallable(
    LIBMATTI_FML_Entrypoint_StartupResult *startupResult, const char *mainClassName);

// Java: startup() and createMainMethodCallable() throw; the C port cannot, so the
// entrypoint's catch clause reads the thrown exception back here.
LIBMATTI_JL_Throwable *LIBMATTI_FML_Entrypoint_GetLastException(void);

// Java: protected static @Nullable Thread findThread(String threadName) - iterates
// Thread.getAllStackTraces() through the ported java.lang.Thread registry
void *LIBMATTI_FML_Entrypoint_FindThread(const char *threadName);

// Java: StartupResult.close() { loader.close(); }
void LIBMATTI_FML_Entrypoint_StartupResult_Close(LIBMATTI_FML_Entrypoint_StartupResult *startupResult);

#endif //MATTICRAFT_FML_ENTRYPOINT_H
