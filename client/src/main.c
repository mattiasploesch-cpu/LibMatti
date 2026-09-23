//
// Created by administrator on 08.09.26.
//

#include "libmatti/main.h"

#include "demo-service.h"
#include "selftest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Manifest of the matticraft artifact, in the ELF section ".matti_manifest" that
// the jar contents reader exposes as "META-INF/MANIFEST.MF". ModLauncher reads
// its own artifact metadata from it (Java: IEnvironment.class.getPackage()
// .getSpecificationVersion() / .getImplementationVersion()).
__attribute__((used, section(".matti_manifest")))
const char MATTICRAFT_MANIFEST[] =
    "Manifest-Version: 1.0\n"
    "Implementation-Version: 11.0\n"
    "Specification-Version: 11.0\n"
    "\n";

// Release packages (AppImage / portable zip) ship with defaults baked in, so
// the game starts with zero arguments: version, launchTarget, the FML version
// markers and the account fields the Java launcher reads from the offline
// profile. An explicit flag on the command line wins over a default.
static const char *arg_for(int argc, char *argv[], const char *flag, const char *fallback)
{
    size_t flen = strlen(flag);
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], flag) == 0 && i + 1 < argc)
            return argv[i + 1];
        if (strncmp(argv[i], flag, flen) == 0 && strncmp(argv[i], flag, strlen(argv[i])) == 0 &&
            strlen(argv[i]) > flen && argv[i][flen] == '=')
            return argv[i] + flen + 1;
    }
    return fallback;
}

static int has_flag(int argc, char *argv[], const char *flag)
{
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], flag) == 0)
            return 1;
    return 0;
}

static void print_help(void)
{
    printf(
        "Matticraft - a from-scratch C port of the Minecraft + NeoForge toolchain\n"
        "\n"
        "Usage: matticraft [options]\n"
        "\n"
        "Launcher options:\n"
        "  --launchTarget <name>   launch service target (default: neoforge; also: minecraft, testharness)\n"
        "  --gameDir <dir>         game directory (default: . ; the packages run it as the game folder)\n"
        "  --version <v>           game version reported to FML (default: 1.21.11)\n"
        "  --accessToken <t>       account access token (default: 0 - the offline profile)\n"
        "  --username <name>       player name (default: Player)\n"
        "  --uuid <uuid>           account uuid (default derived from the username)\n"
        "\n"
        "FML version markers:\n"
        "  --fml.mcVersion <v>         Minecraft version (default: 1.21.11)\n"
        "  --fml.neoForgeVersion <v>   NeoForge version (default: 21.11.45)\n"
        "  --fml.neoFormVersion <v>    NeoForm version (default: 20251209.172050)\n"
        "  --fml.forgeVersion <v>      Forge-line version (default: 45.0)\n"
        "\n"
        "Asset options (unused by the C port, accepted for launcher compatibility):\n"
        "  --assetsDir <dir>       assets directory (default: assets)\n"
        "  --assetIndex <n>        asset index name (default: 1.21)\n"
        "\n"
        "Client options:\n"
        "  --width <n>             window width (default: 854)\n"
        "  --height <n>            window height (default: 480)\n"
        "  --fullscreen            start fullscreen\n"
        "\n"
        "Port options:\n"
        "  --help                  show this help and exit\n"
        "  --no-tests              skip the selftest phase before launching\n"
        "  --no-render             boot without a window (headless)\n"
        "\n"
        "Environment:\n"
        "  MATTI_THEME_FONT=<file>  override the theme font (a Monocraft.ttf copy is embedded)\n"
        "  MATTI_CAM_YAW / MATTI_CAM_PITCH  sweep the skeleton camera (render debugging)\n"
        "  MATTI_CHUNK_DEBUG=1      log the section culling/draw decisions\n"
        "\n"
        "Mods: drop *.matti (or plain *.so) packages into the mods folder of the\n"
        "game directory and restart.\n");
}

int main(int argc, char *argv[], char *envp[])
{
    if (has_flag(argc, argv, "--help") || has_flag(argc, argv, "-h"))
    {
        print_help();
        return 0;
    }

    // The selftest phase: the tests of libmatti live in the client, so one run
    // of matticraft builds and executes every harness (the same commands ctest
    // runs). Skipped with --no-tests, or compiled out for runClient (the plain
    // client target has no test harnesses as dependencies).
#if MATTICRAFT_CLIENT_ONLY
    int runSelftests = 0;
#else
    int runSelftests = 1;
#endif
    if (has_flag(argc, argv, "--no-tests"))
        runSelftests = 0;

    // C has no META-INF/services: declare the demo providers before the
    // launcher reads the registry inside start_from_bsl.
    demo_register_services();

    if (runSelftests && selftest_run_all() != 0)
    {
        fprintf(stderr, "matticraft: selftests failed, not starting the launcher\n");
        fprintf(stderr, "matticraft: run with --no-tests to start anyway\n");
        return 1;
    }

    // Release defaults: the packages start without any manual arguments. An
    // explicitly passed flag wins (arg_for scans the argv first), the defaults
    // are appended after - joption's first-value-wins keeps the explicit ones.
    const char *defaults[] = {
        "--launchTarget", "neoforge",
        "--gameDir", ".",
        "--version", "1.21.11",
        "--fml.mcVersion", "1.21.11",
        "--fml.neoForgeVersion", "21.11.45",
        "--fml.neoFormVersion", "20251209.172050",
        "--fml.forgeVersion", "45.0",
        "--assetsDir", "assets",
        "--assetIndex", "1.21",
        "--accessToken", "0",
        "--username", "Player",
    };
    const int defaultCount = (int) (sizeof(defaults) / sizeof(defaults[0]));

    char **fullArgv = (char **) calloc((size_t) argc + defaultCount + 1, sizeof(char *));
    int fullArgc = 0;
    for (int i = 0; i < argc; i++)
        fullArgv[fullArgc++] = argv[i];
    for (int i = 0; i < defaultCount; i += 2)
    {
        // Only inject a default when the flag (or flag=value form) is absent.
        char flag[64];
        snprintf(flag, sizeof(flag), "%s", defaults[i]);
        if (arg_for(fullArgc, fullArgv, flag, NULL) == NULL)
        {
            fullArgv[fullArgc++] = (char *) defaults[i];
            fullArgv[fullArgc++] = (char *) defaults[i + 1];
        }
    }

    // Java: BootstrapLauncher.run(...) ends in ModLauncher, which hands over to
    // the launch target's game main (net.minecraft.client.main.Main). The
    // process exit status is the game's: start_from_bsl propagates the status
    // the game main returned through the launcher chain.
    int status = start_from_bsl(fullArgc, fullArgv, envp);
    free(fullArgv);
    return status;
}
