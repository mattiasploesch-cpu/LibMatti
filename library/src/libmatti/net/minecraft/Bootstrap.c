// Port of net.minecraft.server.Bootstrap (the bootstrapped guard the registries use).

#include "libmatti/net/minecraft/Bootstrap.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"

#include <stdio.h>
#include <stdlib.h>

// Java: private static volatile boolean isBootstrapped
static int isBootstrapped = 0;

// Java: the bootStrap() body sets the flag; the registry bootstrap raises the guard through
// this setter
void LIBMATTI_MC_Bootstrap_MarkBootstrapped(void)
{
    isBootstrapped = 1;
}

// Java: public static void bootStrap()
void LIBMATTI_MC_Bootstrap_BootStrap(void)
{
    if (isBootstrapped)
        return;
    // Java: isBootstrapped = true comes first - the registry initialisation below passes the
    // guard through internalRegister's checkBootstrapCalled
    isBootstrapped = 1;

    // Java: if (BuiltInRegistries.REGISTRY.keySet().isEmpty()) throw
    //       new IllegalStateException("Unable to load registries") - the reference
    //       class-initialises the registries first; the port runs the constructors explicitly
    LIBMATTI_MC_BuiltInRegistries_InitializeAll();

    size_t rootCount = 0;
    LIBMATTI_MC_MappedRegistry_KeySet(LIBMATTI_MC_BuiltInRegistries_Registry(), &rootCount);
    if (rootCount == 0)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Unable to load registries");
        exit(1);
    }

    // Java: BuiltInRegistries.bootStrap() (createContents, freeze, validate); the game
    //       bootstraps around it are the game port's part
    LIBMATTI_MC_BuiltInRegistries_BootStrap();
}

// Java: public static boolean isBootstrapped()
int LIBMATTI_MC_Bootstrap_IsBootstrapped(void)
{
    return isBootstrapped;
}

// Java: private static RuntimeException createBootstrapException(Supplier<String> location)
// - IllegalArgumentException("Not bootstrapped (called from " + location + ")"); the port
// reports and aborts (Java throws)
void LIBMATTI_MC_Bootstrap_CheckBootstrapCalled(const char *location)
{
    if (isBootstrapped)
        return;
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
    LIBMATTI_ML_Logger_Error(logger, NULL, "Not bootstrapped (called from {})", location);
    exit(1);
}
