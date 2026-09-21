#include "libmatti/net/neoforged/fml/startup/Client.h"

#include "libmatti/java/lang/System.h"
#include "libmatti/net/minecraft/client/main/Main.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"
#include "libmatti/net/neoforged/neoforge/client/loading/ClientModLoader.h"
#include "libmatti/net/neoforged/fml/startup/FatalErrorReporting.h"

#include <stdint.h>
#include <stdlib.h>

// Java: private Client() {}
LIBMATTI_FML_Client *LIBMATTI_FML_Client_New(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_Client));
}

void LIBMATTI_FML_Client_Free(LIBMATTI_FML_Client *client)
{
    free(client);
}

// Java: public static void main(String[] args)
// The port returns the game's exit status; every error path below keeps
// Java's "report and exit(1)" behaviour.
int LIBMATTI_FML_Client_Main(int argc, char *argv[])
{
    // Java: try (var startupResult = startup(args, false, Dist.CLIENT, true)) {
    LIBMATTI_FML_Entrypoint_StartupResult *startupResult =
        LIBMATTI_FML_Entrypoint_Startup(argc, argv, 0, LIBMATTI_DIST_CLIENT, 1);

    // Java: startup(...) threw -> the catch clause below runs
    if (startupResult == NULL)
    {
        // Java: catch (Throwable t) { FatalErrorReporting.reportFatalError(t); System.exit(1); }
        LIBMATTI_FML_FatalErrorReporting_ReportFatalError(LIBMATTI_FML_Entrypoint_GetLastException());
        LIBMATTI_JL_System_Exit(1);
        return 1;
    }

    // Java: net.neoforged.neoforge.client.loading.ClientModLoader.begin();
    // (Main.java.patch: "Mirroring server mod-loading, we construct immediately after Bootstrap.validate()")
    LIBMATTI_NEOFORGE_ClientModLoader_Begin();

    // Java: the game shows the error screen instead of continuing; the port reports through the
    // fatal error path, like the ModLoadingException branch of ClientModLoader.begin would.
    if (LIBMATTI_FML_ModLoader_HasErrors())
    {
        LIBMATTI_FML_Entrypoint_StartupResult_Close(startupResult);
        LIBMATTI_FML_FatalErrorReporting_ReportFatalError(LIBMATTI_FML_ModLoader_GetLastException());
        LIBMATTI_JL_System_Exit(1);
        return 1;
    }

    // Java: Minecraft's constructor runs, then the first resource reload triggers
    // ClientModLoader.onResourceReload -> load() + finishModLoading(). The game (window, resource
    // reload) is the port's missing part; until it exists the reload stages run here, which keeps
    // the Java order (setup events after construction, completeModLoading before the title screen).
    // TODO: remove once Minecraft.getInstance() exists and binds the reload listener itself.
    LIBMATTI_NEOFORGE_ClientModLoader_OnResourceReload();

    // Java: runnable = ClientModLoader.completeModLoading(runnable) in Minecraft.java.patch
    void (*initialScreensTask)(void *userdata) = NULL;
    void (*runnable)(void *userdata) =
        LIBMATTI_NEOFORGE_ClientModLoader_CompleteModLoading(initialScreensTask);

    // Java: the title screen would run the reload runnable here; the port runs it directly (the
    // task is NULL or the game port's screens task) before the game main takes over.
    if (runnable != NULL) runnable(NULL);

    // Java: var main = createMainMethodCallable(startupResult, "net.minecraft.client.main.Main");
    LIBMATTI_JLI_MethodHandle *main =
        LIBMATTI_FML_Entrypoint_CreateMainMethodCallable(startupResult, "net.minecraft.client.main.Main");

    // Java: createMainMethodCallable(...) threw -> try-with-resources closes, then the catch clause runs
    if (main == NULL)
    {
        LIBMATTI_FML_Entrypoint_StartupResult_Close(startupResult);
        // Java: catch (Throwable t) { FatalErrorReporting.reportFatalError(t); System.exit(1); }
        LIBMATTI_FML_FatalErrorReporting_ReportFatalError(LIBMATTI_FML_Entrypoint_GetLastException());
        LIBMATTI_JL_System_Exit(1);
        return 1;
    }

    // Java: main.invokeExact(startupResult.loader().getProgramArgs().getArguments());
    size_t argumentCount = 0;
    char **arguments = LIBMATTI_FML_ProgramArgs_GetArguments(
        LIBMATTI_FML_FMLLoader_GetProgramArgs(startupResult->loader), &argumentCount);

    // The port's game main keeps the process entry point signature and its
    // return value is the game's exit status (Java's main is void and the JVM
    // would exit 0 once it returns).
    int exitCode = (int) (intptr_t) LIBMATTI_JLI_MethodHandle_InvokeExact(main, (int) argumentCount, arguments);

    // Java leaves the array to the garbage collector; the C port releases it once the target returned.
    for (size_t i = 0; i < argumentCount; i++)
        free(arguments[i]);
    free(arguments);

    // Java: try-with-resources -> StartupResult.close() (FMLLoader.close: "Closing FML Loader")
    LIBMATTI_FML_Entrypoint_StartupResult_Close(startupResult);

    return exitCode;
}
