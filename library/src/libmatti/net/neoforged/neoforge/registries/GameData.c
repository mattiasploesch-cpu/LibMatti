// Port of net.neoforged.neoforge.registries.GameData.

#include "libmatti/net/neoforged/neoforge/registries/GameData.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"
#include "libmatti/net/neoforged/neoforge/registries/RegisterEvent.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **rootRegistries = NULL;
static size_t rootRegistryCount = 0;

// Java: the frozen state of the mapped registries
static int frozen = 1;

static void add_name(char ***names, size_t *count, const char *name)
{
    *names = realloc(*names, sizeof(char *) * (*count + 1));
    (*names)[(*count)++] = strdup(name);
}

static int contains_name(char **names, size_t count, const char *name)
{
    for (size_t i = 0; i < count; i++)
        if (strcmp(names[i], name) == 0) return 1;
    return 0;
}

void LIBMATTI_NEOFORGE_GameData_AddRootRegistry(const char *registryName)
{
    if (!contains_name(rootRegistries, rootRegistryCount, registryName))
        add_name(&rootRegistries, &rootRegistryCount, registryName);
}

// Java: public static Set<Identifier> getRegistrationOrder()
char **LIBMATTI_NEOFORGE_GameData_GetRegistrationOrder(size_t *count)
{
    // Java: Attributes first (Item and MobEffect depend on it at construction time)
    static const char *VANILLA_FIRST[] = {
        "minecraft:attribute",          // Registries.ATTRIBUTE
        "minecraft:data_component_type", // Registries.DATA_COMPONENT_TYPE
        "minecraft:particle_type",      // Registries.PARTICLE_TYPE
        NULL};

    char **ordered = NULL;
    size_t orderedCount = 0;

    for (int i = 0; VANILLA_FIRST[i] != NULL; i++)
    {
        if (contains_name(rootRegistries, rootRegistryCount, VANILLA_FIRST[i]))
            add_name(&ordered, &orderedCount, VANILLA_FIRST[i]);
    }

    // Java: BuiltInRegistries.getVanillaRegistrationOrder() - the remaining vanilla registries
    for (size_t i = 0; i < rootRegistryCount; i++)
    {
        const char *name = rootRegistries[i];
        int alreadyAdded = 0;
        for (int j = 0; VANILLA_FIRST[j] != NULL; j++)
            if (strcmp(name, VANILLA_FIRST[j]) == 0)
            {
                alreadyAdded = 1;
                break;
            }
        if (alreadyAdded) continue;

        // Java: modded registries come after the vanilla ones, alphabetically; the port sorts
        // the whole remainder by name (minecraft: before every other namespace is the Java
        // compareNamespaced behaviour for the built-in set)
        add_name(&ordered, &orderedCount, name);
    }

    *count = orderedCount;
    return ordered;
}

// Java: public static void unfreezeData()
void LIBMATTI_NEOFORGE_GameData_UnfreezeData(void)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Unfreezing registries");
    frozen = 0;
    // Java: BuiltInRegistries.REGISTRY.stream().filter(r -> r instanceof BaseMappedRegistry)
    //       .forEach(r -> ((BaseMappedRegistry<?>) r).unfreeze(true)) - every built-in registry
    //       lifts its write guard
    size_t count = 0;
    LIBMATTI_MC_ResourceKey **keys = LIBMATTI_MC_MappedRegistry_RegistryKeySet(
        LIBMATTI_MC_BuiltInRegistries_Registry(), &count);
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_MappedRegistry *entry =
            LIBMATTI_MC_MappedRegistry_GetValueByKey(LIBMATTI_MC_BuiltInRegistries_Registry(), keys[i]);
        if (entry != NULL)
            LIBMATTI_MC_MappedRegistry_Unfreeze(entry);
    }
}

// Java: public static void freezeData()
void LIBMATTI_NEOFORGE_GameData_FreezeData(void)
{
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Freezing registries");
    // Java: BuiltInRegistries.REGISTRY.stream().filter(r -> r instanceof MappedRegistry).forEach(r -> {
    //       ((MappedRegistry<?>) r).bindAllTagsToEmpty(); ((MappedRegistry<?>) r).freeze(); })
    size_t count = 0;
    LIBMATTI_MC_ResourceKey **keys = LIBMATTI_MC_MappedRegistry_RegistryKeySet(
        LIBMATTI_MC_BuiltInRegistries_Registry(), &count);
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_MappedRegistry *entry =
            LIBMATTI_MC_MappedRegistry_GetValueByKey(LIBMATTI_MC_BuiltInRegistries_Registry(), keys[i]);
        if (entry != NULL)
            LIBMATTI_MC_MappedRegistry_Freeze(entry);
    }
    // Java: RegistryManager.takeFrozenSnapshot() - the snapshot registry model is the game port's
    //       part
    LIBMATTI_ML_Logger_Debug(LIBMATTI_ML_LogManager_GetLogger(), NULL, "All registries frozen");
    frozen = 1;
}

// Java: public static void postRegisterEvents()
void LIBMATTI_NEOFORGE_GameData_PostRegisterEvents(void)
{
    size_t orderedCount = 0;
    char **ordered = LIBMATTI_NEOFORGE_GameData_GetRegistrationOrder(&orderedCount);

    for (size_t i = 0; i < orderedCount; i++)
    {
        // Java: StartupNotificationManager.modLoaderMessage("REGISTERING " + registryKey.identifier())
        char message[256];
        snprintf(message, sizeof(message), "REGISTERING %s", ordered[i]);
        LIBMATTI_FML_StartupNotificationManager_ModLoaderMessage(message);

        // Java: the events get the real registry object - look it up in BuiltInRegistries.REGISTRY
        LIBMATTI_MC_Identifier *location = LIBMATTI_MC_Identifier_Parse(ordered[i]);
        LIBMATTI_MC_MappedRegistry *registry =
            LIBMATTI_MC_MappedRegistry_GetValue(LIBMATTI_MC_BuiltInRegistries_Registry(), location);
        LIBMATTI_MC_Identifier_Free(location);

        LIBMATTI_NEOFORGE_Registries_RegisterEvent *event =
            LIBMATTI_NEOFORGE_Registries_RegisterEvent_NewFull(ordered[i], registry);
        LIBMATTI_FML_ModLoader_PostEventWrapContainerInModOrder(&event->base);
        LIBMATTI_NEOFORGE_Registries_RegisterEvent_Free(event);
    }

    for (size_t i = 0; i < orderedCount; i++)
        free(ordered[i]);
    free(ordered);

    (void) frozen;
}
