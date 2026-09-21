// Port of net.neoforged.fml.ModContainer.

#include "libmatti/net/neoforged/fml/ModContainer.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/config/ConfigTracker.h"
#include "libmatti/net/neoforged/fml/config/IConfigSpec.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: public ModContainer(IModInfo info)
// Java's field initialisers run before the constructor body, so the extension map starts out empty.
void LIBMATTI_FML_ModContainer_Init(LIBMATTI_FML_ModContainer *container, LIBMATTI_NEOFORGESPI_IModInfo *info)
{
    container->extensionPoints = NULL;
    container->extensionPointCount = 0;

    container->modId = strdup(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(info));
    // Java (the source carries the same TODO upstream): // TODO: Currently not reading namespace from configuration..
    //       this.namespace = this.modId;
    container->namespace = strdup(container->modId);
    container->modInfo = info;
}

void LIBMATTI_FML_ModContainer_Free(LIBMATTI_FML_ModContainer *container)
{
    if (container == NULL) return;
    if (container->free != NULL) container->free(container);

    free(container->modId);
    free(container->namespace);
    free(container->extensionPoints);
    free(container);
}

// Java: public final String getModId()
const char *LIBMATTI_FML_ModContainer_GetModId(const LIBMATTI_FML_ModContainer *container)
{
    return container->modId;
}

// Java: public final String getNamespace()
const char *LIBMATTI_FML_ModContainer_GetNamespace(const LIBMATTI_FML_ModContainer *container)
{
    return container->namespace;
}

// Java: public IModInfo getModInfo()
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModContainer_GetModInfo(const LIBMATTI_FML_ModContainer *container)
{
    return container->modInfo;
}

// Java: extensionPoints.getOrDefault(point, () -> null).get()
void *LIBMATTI_FML_ModContainer_GetCustomExtension(const LIBMATTI_FML_ModContainer *container,
                                                  LIBMATTI_FML_IExtensionPoint point)
{
    for (size_t i = 0; i < container->extensionPointCount; i++)
    {
        const LIBMATTI_FML_ModContainer_ExtensionPoint *entry = &container->extensionPoints[i];
        if (strcmp(entry->point, point) != 0) continue;

        return entry->supplier.get != NULL ? LIBMATTI_JU_Supplier_Get(&entry->supplier) : entry->value;
    }

    return NULL;
}

// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, T extension)
void LIBMATTI_FML_ModContainer_RegisterExtensionPoint(LIBMATTI_FML_ModContainer *container,
                                                     LIBMATTI_FML_IExtensionPoint point, void *extension)
{
    container->extensionPoints =
        realloc(container->extensionPoints, sizeof(*container->extensionPoints) * (container->extensionPointCount + 1));

    LIBMATTI_FML_ModContainer_ExtensionPoint *entry =
        &container->extensionPoints[container->extensionPointCount++];
    entry->point = point;
    entry->supplier.self = NULL;
    entry->supplier.get = NULL;
    entry->value = extension;
}

// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, Supplier<T> extension)
void LIBMATTI_FML_ModContainer_RegisterExtensionPointSupplier(LIBMATTI_FML_ModContainer *container,
                                                             LIBMATTI_FML_IExtensionPoint point,
                                                             LIBMATTI_JU_Supplier supplier)
{
    container->extensionPoints =
        realloc(container->extensionPoints, sizeof(*container->extensionPoints) * (container->extensionPointCount + 1));

    LIBMATTI_FML_ModContainer_ExtensionPoint *entry =
        &container->extensionPoints[container->extensionPointCount++];
    entry->point = point;
    entry->supplier = supplier;
    entry->value = NULL;
}

// Java: public void registerConfig(ModConfig.Type type, IConfigSpec configSpec) {
//           if (configSpec.isEmpty()) { LOGGER.debug(LOADING, "Skipping config registration for mod {} because no config entries exist", ...); return; }
//           ConfigTracker.INSTANCE.registerConfig(type, configSpec, this); }
void LIBMATTI_FML_ModContainer_RegisterConfig(LIBMATTI_FML_ModContainer *container, int type, void *configSpec)
{
    LIBMATTI_FML_IConfigSpec *spec = configSpec;

    if (spec->isEmpty(spec->self))
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Skipping config registration for mod {} because no config entries exist",
                                 container->modId);
        return;
    }

    LIBMATTI_FML_ConfigTracker_RegisterConfig(LIBMATTI_FML_ConfigTracker_Instance(),
                                              (LIBMATTI_FML_ModConfig_Type) type, spec, container);
}

// Java: public void registerConfig(ModConfig.Type type, IConfigSpec configSpec, String fileName)
void LIBMATTI_FML_ModContainer_RegisterConfigNamed(LIBMATTI_FML_ModContainer *container, int type, void *configSpec,
                                                  const char *fileName)
{
    LIBMATTI_FML_IConfigSpec *spec = configSpec;

    if (spec->isEmpty(spec->self))
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Skipping config registration for mod {} because no config entries exist",
                                 container->modId);
        return;
    }

    LIBMATTI_FML_ConfigTracker_RegisterConfigNamed(LIBMATTI_FML_ConfigTracker_Instance(),
                                                   (LIBMATTI_FML_ModConfig_Type) type, spec, container, fileName);
}

// Java: @Nullable public abstract IEventBus getEventBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_ModContainer_GetEventBus(LIBMATTI_FML_ModContainer *container)
{
    if (container->getEventBus == NULL) return NULL;
    return container->getEventBus(container);
}

// Java: @ApiStatus.OverrideOnly protected void constructMod() {}
void LIBMATTI_FML_ModContainer_ConstructMod(LIBMATTI_FML_ModContainer *container)
{
    if (container->constructMod != NULL) container->constructMod(container);
}

// Java: public final <T extends Event & IModBusEvent> void acceptEvent(T e)
void LIBMATTI_FML_ModContainer_AcceptEvent(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_IEventBus *bus = LIBMATTI_FML_ModContainer_GetEventBus(container);
    if (bus == NULL) return;

    // Java: try { LOGGER.trace(LOADING, "Firing event for modid {} : {}", getModId(), e); bus.post(e);
    //              LOGGER.trace(LOADING, "Fired event for modid {} : {}", getModId(), e); }
    //       catch (Throwable t) { ... throw new ModLoadingException(...) }
    // A C listener does not throw into the bus (see EventBus.post), so there is nothing to catch.
    //LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Firing event for modid {} : {}",
    //                         container->modId, LIBMATTI_JL_Class_GetName(event->clazz));
    LIBMATTI_BUS_IEventBus_Post(bus, event);
    //LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Fired event for modid {} : {}",
    //                         container->modId, LIBMATTI_JL_Class_GetName(event->clazz));
}

// Java: public final <T extends Event & IModBusEvent> void acceptEvent(EventPriority phase, T e)
void LIBMATTI_FML_ModContainer_AcceptEventWithPhase(LIBMATTI_FML_ModContainer *container,
                                                   LIBMATTI_BUS_EventPriority phase, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_IEventBus *bus = LIBMATTI_FML_ModContainer_GetEventBus(container);
    if (bus == NULL) return;

    //LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Firing event for phase {} for modid {} : {}",
    //                         LIBMATTI_BUS_EventPriority_Name(phase), container->modId,
    //                         LIBMATTI_JL_Class_GetName(event->clazz));
    LIBMATTI_BUS_IEventBus_PostPhase(bus, phase, event);
    //LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Fired event for phase {} for modid {} : {}",
    //                         LIBMATTI_BUS_EventPriority_Name(phase), container->modId,
    //                         LIBMATTI_JL_Class_GetName(event->clazz));
}
