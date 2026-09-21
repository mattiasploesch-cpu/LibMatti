// Port of net.minecraft.client.main.Main (the game class FML hands over to).
// See Main.h - the class registers on the game content classloader; main parses
// the FML program arguments into a GameConfig like joptsimple does in Java and
// runs the Minecraft skeleton (window + run loop).

#include "libmatti/net/minecraft/client/main/Main.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/net/minecraft/client/Minecraft.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: Main.java joptsimple parsing. The options the skeleton consumes carry a
// value (name=value); flags stand alone.
static const char *optionValue(int argc, char *argv[], const char *name, const char *fallback)
{
    size_t nameLen = strlen(name);
    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], name, nameLen) == 0 && argv[i][nameLen] == '=')
            return argv[i] + nameLen + 1;
    }
    return fallback;
}

static int optionFlag(int argc, char *argv[], const char *name)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], name) == 0)
            return 1;
    }
    return 0;
}

static int mc_client_main(int argc, char *argv[])
{
    // Java: RenderSystem.initRenderThread(); - the first call of Main.main
    // (before any GL work, like the Java entry point).
    LIBMATTI_B3D_RenderSystem_InitRenderThread();

    // The tick dispatch lives in Minecraft.tick (the loop calls it); main
    // itself only parses, constructs and runs like Java's Main.main.
    // Java: new GameConfig(..., new DisplayData(i, j, optionalint, optionalint1, flag), ...)
    // Defaults: width 854, height 480 (optionspec22/23 defaultsTo).
    LIBMATTI_MC_GameConfig config;
    memset(&config, 0, sizeof(config));

    config.display.width = atoi(optionValue(argc, argv, "--width", "854"));
    config.display.height = atoi(optionValue(argc, argv, "--height", "480"));
    if (optionFlag(argc, argv, "--fullscreenWidth"))
    {
        config.display.fullscreenWidth = atoi(optionValue(argc, argv, "--fullscreenWidth", "0"));
        config.display.fullscreenWidthPresent = 1;
    }
    if (optionFlag(argc, argv, "--fullscreenHeight"))
    {
        config.display.fullscreenHeight = atoi(optionValue(argc, argv, "--fullscreenHeight", "0"));
        config.display.fullscreenHeightPresent = 1;
    }
    config.display.isFullscreen = optionFlag(argc, argv, "--fullscreen");

    config.game.demo = optionFlag(argc, argv, "--demo");
    config.game.launchVersion = optionValue(argc, argv, "--version", "1.21.11");
    config.game.versionType = optionValue(argc, argv, "--versionType", "release");
    config.game.disableMultiplayer = optionFlag(argc, argv, "--disableMultiplayer");
    config.game.disableChat = optionFlag(argc, argv, "--disableChat");

    config.location.gameDirectory = optionValue(argc, argv, "--gameDir", ".");
    config.location.assetDirectory = optionValue(argc, argv, "--assetsDir", "assets/");
    config.location.resourcePackDirectory = optionValue(argc, argv, "--resourcePackDir", "resourcepacks/");
    config.location.assetIndex = optionValue(argc, argv, "--assetIndex", NULL);

    config.user.name = optionValue(argc, argv, "--username", "23TheCoolBear45");
    config.user.uuid = optionValue(argc, argv, "--uuid", "");
    config.user.accessToken = optionValue(argc, argv, "--accessToken", "");

    config.quickPlayLogPath = optionValue(argc, argv, "--quickPlayPath", NULL);
    config.quickPlayWorld = optionValue(argc, argv, "--quickPlaySingleplayer", NULL);

    // Java: Minecraft minecraft = new Minecraft(gameconfig); minecraft.run();
    // The demo mixin target fires once per frame like the game's tick body.
    LIBMATTI_MC_Minecraft *minecraft = LIBMATTI_MC_Minecraft_New(&config);
    if (minecraft == NULL)
        return 1;

    LIBMATTI_MC_Minecraft_Run(minecraft);
    LIBMATTI_MC_Minecraft_Destroy(minecraft);

    // Java: main returns void; the JVM exits 0. The port returns the loop's
    // end state as the process exit code.
    return 0;
}

// Java: the JVM resolves the static method by name; the port forwards main to
// the registered body so the harness can call either.
int LIBMATTI_MC_Client_Main_Main(int argc, char *argv[])
{
    return mc_client_main(argc, argv);
}

void LIBMATTI_MC_Client_Main_Register(void *classLoader)
{
    LIBMATTI_JL_Class *mainClass =
        LIBMATTI_JL_Class_RegisterClass("net.minecraft.client.main.Main", classLoader, NULL, 1);
    LIBMATTI_JL_Class_RegisterStaticMethod(mainClass, "main", (void (*)(int, char *[])) LIBMATTI_MC_Client_Main_Main);
}
