// Port of net.neoforged.neoforge.registries.RegisterEvent.
// Java carries the ResourceKey and the Registry object; the port keeps the registry name (the
// ResourceKey's identifier) and lets mods register into the name->entry table of that registry.
// The Registry objects themselves are the game port's part (BuiltInRegistries is Minecraft).

#ifndef MATTICRAFT_NEOFORGE_REGISTRIES_REGISTEREVENT_H
#define MATTICRAFT_NEOFORGE_REGISTRIES_REGISTEREVENT_H

#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/neoforged/bus/api/Event.h"

typedef struct LIBMATTI_NEOFORGE_Registries_RegisterEvent LIBMATTI_NEOFORGE_Registries_RegisterEvent;

// Java: public class RegisterEvent<T> extends Event implements IModBusEvent
struct LIBMATTI_NEOFORGE_Registries_RegisterEvent
{
    LIBMATTI_BUS_Event base;
    // Java: private final ResourceKey<? extends Registry<T>> name;
    char *registryName;
    // Java: private final Registry<T> registry - the real registry object (BuiltInRegistries)
    LIBMATTI_MC_MappedRegistry *registry;
    // Java: the entries registered into the registry through this event (name -> entry)
    char **entryNames;
    void **entries;
    size_t entryCount;
    size_t entryCapacity;
};

// Java: RegisterEvent.class
LIBMATTI_JL_Class *LIBMATTI_NEOFORGE_Registries_RegisterEvent_Class(void);

// Java: public RegisterEvent(ResourceKey<? extends Registry<T>> name, Registry<T> registry)
LIBMATTI_NEOFORGE_Registries_RegisterEvent *LIBMATTI_NEOFORGE_Registries_RegisterEvent_NewFull(
    const char *registryName, LIBMATTI_MC_MappedRegistry *registry);
// convenience without the registry object (the event cannot write)
LIBMATTI_NEOFORGE_Registries_RegisterEvent *LIBMATTI_NEOFORGE_Registries_RegisterEvent_New(const char *registryName);
void LIBMATTI_NEOFORGE_Registries_RegisterEvent_Free(LIBMATTI_NEOFORGE_Registries_RegisterEvent *event);

// Java: public <T> void register(ResourceLocation key, T value) - the entry names the event collected
void LIBMATTI_NEOFORGE_Registries_RegisterEvent_Register(LIBMATTI_NEOFORGE_Registries_RegisterEvent *event,
                                                        const char *entryName, void *entry);

#endif //MATTICRAFT_NEOFORGE_REGISTRIES_REGISTEREVENT_H
