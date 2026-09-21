// Port of net.neoforged.neoforge.registries.RegisterEvent.

#include "libmatti/net/neoforged/neoforge/registries/RegisterEvent.h"

#include "libmatti/net/minecraft/core/Registry.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Registries_RegisterEvent_Class(void)
{
    LIBMATTI_JL_Class *clazz =
        LIBMATTI_JL_Class_RegisterClass("net.neoforged.neoforge.registries.RegisterEvent", NULL,
                                        LIBMATTI_BUS_Event_Class(), LIBMATTI_JL_Modifier_PUBLIC);
    LIBMATTI_JL_Class_AddInterface(clazz, LIBMATTI_FML_Event_IModBusEvent_Class());
    return clazz;
}

// Java: public RegisterEvent(ResourceKey<? extends Registry<T>> name, Registry<T> registry)
LIBMATTI_NEOFORGE_Registries_RegisterEvent *LIBMATTI_NEOFORGE_Registries_RegisterEvent_NewFull(
    const char *registryName, LIBMATTI_MC_MappedRegistry *registry)
{
    LIBMATTI_NEOFORGE_Registries_RegisterEvent *event =
        calloc(1, sizeof(LIBMATTI_NEOFORGE_Registries_RegisterEvent));
    LIBMATTI_BUS_Event_Init(&event->base, LIBMATTI_NEOFORGE_Registries_RegisterEvent_Class());
    event->registryName = strdup(registryName);
    event->registry = registry;
    return event;
}

LIBMATTI_NEOFORGE_Registries_RegisterEvent *LIBMATTI_NEOFORGE_Registries_RegisterEvent_New(const char *registryName)
{
    return LIBMATTI_NEOFORGE_Registries_RegisterEvent_NewFull(registryName, NULL);
}

// Java: public <T> void register(ResourceKey<? extends Registry<T>> registryKey, Identifier name,
//       Supplier<T> valueSupplier) - Registry.register(this.registry, name, value.get()); the
// port's caller filters on the registry name before, so the entry writes into the real registry
void LIBMATTI_NEOFORGE_Registries_RegisterEvent_Register(LIBMATTI_NEOFORGE_Registries_RegisterEvent *event,
                                                        const char *entryName, void *entry)
{
    if (event->registry != NULL)
    {
        LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(entryName);
        LIBMATTI_MC_Registry_RegisterWithIdentifier(event->registry, identifier, entry);
        LIBMATTI_MC_Identifier_Free(identifier);
    }
    if (event->entryCount == event->entryCapacity)
    {
        event->entryCapacity = event->entryCapacity > 0 ? event->entryCapacity * 2 : 8;
        event->entryNames = realloc(event->entryNames, sizeof(char *) * event->entryCapacity);
        event->entries = realloc(event->entries, sizeof(void *) * event->entryCapacity);
    }
    event->entryNames[event->entryCount] = strdup(entryName);
    event->entries[event->entryCount] = entry;
    event->entryCount++;
}

void LIBMATTI_NEOFORGE_Registries_RegisterEvent_Free(LIBMATTI_NEOFORGE_Registries_RegisterEvent *event)
{
    if (event == NULL) return;
    for (size_t i = 0; i < event->entryCount; i++)
        free(event->entryNames[i]);
    free(event->entryNames);
    free(event->entries);
    free(event->registryName);
    free(event);
}
