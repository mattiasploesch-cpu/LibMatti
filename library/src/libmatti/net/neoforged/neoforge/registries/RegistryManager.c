// Port of net.neoforged.neoforge.registries.RegistryManager.
// The event lifecycle runs exactly like Java; the Registry object model (BuiltInRegistries)
// is the game port's part, so the port tracks registry names instead of Registry objects.

#include "libmatti/net/neoforged/neoforge/registries/RegistryManager.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/registries/ModifyRegistriesEvent.h"

#include <stdlib.h>
#include <string.h>

static char **pendingModdedRegistries = NULL;
static size_t pendingModdedRegistryCount = 0;
static int pendingModdedRegistriesOpen = 0; // Java: pendingModdedRegistries != null

static char **vanillaRegistryKeys = NULL;
static size_t vanillaRegistryKeyCount = 0;

static char **dataMapTypes = NULL;
static size_t dataMapTypeCount = 0;

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

// Java: public static void postNewRegistryEvent()
void LIBMATTI_NEOFORGE_RegistryManager_PostNewRegistryEvent(void)
{
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();

    LIBMATTI_NEOFORGE_Registries_NewRegistryEvent *event = LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_New();
    // Java: DataPackRegistryEvent.NewRegistry dataPackEvent - posted in the same order below

    // Java: vanillaRegistryKeys = Set.copyOf(BuiltInRegistries.REGISTRY.keySet())
    pendingModdedRegistriesOpen = 1;

    // Java: ModLoader.postEventWrapContainerInModOrder(event)
    LIBMATTI_FML_ModLoader_PostEventWrapContainerInModOrder(&event->base);

    // Java: event.fill() - the registered registries leave the pending set
    LIBMATTI_NEOFORGE_RegistryManager_ClearPendingModdedRegistries();

    // Java: ModLoader.postEvent(new ModifyRegistriesEvent())
    LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent *modifyEvent =
        LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent_New();
    LIBMATTI_FML_ModLoader_PostEvent(&modifyEvent->base);
    LIBMATTI_NEOFORGE_Registries_ModifyRegistriesEvent_Free(modifyEvent);

    LIBMATTI_NEOFORGE_Registries_NewRegistryEvent_Free(event);
    (void) logger;
}

// Java: public static void initDataMaps()
void LIBMATTI_NEOFORGE_RegistryManager_InitDataMaps(void)
{
    // Java: ModLoader.postEvent(new RegisterDataMapTypesEvent(dataMapTypes)); the port keeps the
    // collected type names (the data map payload model is the game port's part)
    for (size_t i = 0; i < dataMapTypeCount; i++)
        free(dataMapTypes[i]);
    free(dataMapTypes);
    dataMapTypes = NULL;
    dataMapTypeCount = 0;

    (void) add_name;
    (void) contains_name;
}

void LIBMATTI_NEOFORGE_RegistryManager_AddPendingModdedRegistry(const char *registryName)
{
    if (pendingModdedRegistriesOpen && !contains_name(pendingModdedRegistries, pendingModdedRegistryCount, registryName))
        add_name(&pendingModdedRegistries, &pendingModdedRegistryCount, registryName);
}

// Java: pendingModdedRegistries.removeIf(BuiltInRegistries.REGISTRY::containsKey) + the
// IllegalStateException for the leftovers; the port clears and reports
void LIBMATTI_NEOFORGE_RegistryManager_ClearPendingModdedRegistries(void)
{
    for (size_t i = 0; i < pendingModdedRegistryCount; i++)
        free(pendingModdedRegistries[i]);
    free(pendingModdedRegistries);
    pendingModdedRegistries = NULL;
    pendingModdedRegistryCount = 0;
    pendingModdedRegistriesOpen = 0;
}

void LIBMATTI_NEOFORGE_RegistryManager_AddVanillaRegistryKey(const char *registryName)
{
    if (!contains_name(vanillaRegistryKeys, vanillaRegistryKeyCount, registryName))
        add_name(&vanillaRegistryKeys, &vanillaRegistryKeyCount, registryName);
}

int LIBMATTI_NEOFORGE_RegistryManager_IsVanillaRegistryKey(const char *registryName)
{
    return contains_name(vanillaRegistryKeys, vanillaRegistryKeyCount, registryName);
}
