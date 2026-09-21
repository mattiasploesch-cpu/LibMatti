// Port of net.neoforged.fml.ModContainer.
// Java's abstract getEventBus() and the overridable constructMod() become the function pointers the
// concrete container fills in (the same convention the port uses for the other abstract classes).

#ifndef MATTICRAFT_FML_MODCONTAINER_H
#define MATTICRAFT_FML_MODCONTAINER_H

#include "libmatti/java/util/function/Supplier.h"
#include "libmatti/net/neoforged/bus/api/Event.h"
#include "libmatti/net/neoforged/bus/api/EventPriority.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/fml/IExtensionPoint.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_ModContainer LIBMATTI_FML_ModContainer;

// Java: Map<Class<? extends IExtensionPoint>, Supplier<?>> extensionPoints (IdentityHashMap)
// The port keys by the extension point's name because IExtensionPoint is a name there.
typedef struct
{
    LIBMATTI_FML_IExtensionPoint point;
    // Java: Supplier<?> - with a NULL get the entry holds the extension directly
    // (Java: registerExtensionPoint(point, () -> extension))
    LIBMATTI_JU_Supplier supplier;
    void *value;
} LIBMATTI_FML_ModContainer_ExtensionPoint;

// Java: public abstract class ModContainer
struct LIBMATTI_FML_ModContainer
{
    // Java: the VM's virtual dispatch
    void (*constructMod)(void *self);
    LIBMATTI_BUS_IEventBus *(*getEventBus)(void *self);
    void (*free)(void *self);

    // Java: protected final String modId
    char *modId;
    // Java: protected final String namespace
    char *namespace;
    // Java: protected final IModInfo modInfo
    LIBMATTI_NEOFORGESPI_IModInfo *modInfo;
    LIBMATTI_FML_ModContainer_ExtensionPoint *extensionPoints;
    size_t extensionPointCount;
};

// Java: public ModContainer(IModInfo info)
void LIBMATTI_FML_ModContainer_Init(LIBMATTI_FML_ModContainer *container, LIBMATTI_NEOFORGESPI_IModInfo *info);
void LIBMATTI_FML_ModContainer_Free(LIBMATTI_FML_ModContainer *container);

// Java: public final String getModId()
const char *LIBMATTI_FML_ModContainer_GetModId(const LIBMATTI_FML_ModContainer *container);
// Java: public final String getNamespace()
const char *LIBMATTI_FML_ModContainer_GetNamespace(const LIBMATTI_FML_ModContainer *container);
// Java: public IModInfo getModInfo()
LIBMATTI_NEOFORGESPI_IModInfo *LIBMATTI_FML_ModContainer_GetModInfo(const LIBMATTI_FML_ModContainer *container);

// Java: public <T extends IExtensionPoint> Optional<T> getCustomExtension(Class<T> point) - NULL when absent
void *LIBMATTI_FML_ModContainer_GetCustomExtension(const LIBMATTI_FML_ModContainer *container,
                                                  LIBMATTI_FML_IExtensionPoint point);
// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, T extension)
void LIBMATTI_FML_ModContainer_RegisterExtensionPoint(LIBMATTI_FML_ModContainer *container,
                                                     LIBMATTI_FML_IExtensionPoint point, void *extension);
// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, Supplier<T> extension)
void LIBMATTI_FML_ModContainer_RegisterExtensionPointSupplier(LIBMATTI_FML_ModContainer *container,
                                                             LIBMATTI_FML_IExtensionPoint point,
                                                             LIBMATTI_JU_Supplier supplier);

// Java: public void registerConfig(ModConfig.Type type, IConfigSpec configSpec)
// 'configSpec' is the spec implementation's self pointer registered through
// LIBMATTI_FML_IConfigSpec_Of (Java's interface reference).
void LIBMATTI_FML_ModContainer_RegisterConfig(LIBMATTI_FML_ModContainer *container, int type, void *configSpec);
// Java: public void registerConfig(ModConfig.Type type, IConfigSpec configSpec, String fileName)
void LIBMATTI_FML_ModContainer_RegisterConfigNamed(LIBMATTI_FML_ModContainer *container, int type, void *configSpec,
                                                  const char *fileName);

// Java: @Nullable public abstract IEventBus getEventBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_ModContainer_GetEventBus(LIBMATTI_FML_ModContainer *container);
// Java: @ApiStatus.OverrideOnly protected void constructMod()
void LIBMATTI_FML_ModContainer_ConstructMod(LIBMATTI_FML_ModContainer *container);

// Java: public final <T extends Event & IModBusEvent> void acceptEvent(T e)
void LIBMATTI_FML_ModContainer_AcceptEvent(LIBMATTI_FML_ModContainer *container, LIBMATTI_BUS_Event *event);
// Java: public final <T extends Event & IModBusEvent> void acceptEvent(EventPriority phase, T e)
void LIBMATTI_FML_ModContainer_AcceptEventWithPhase(LIBMATTI_FML_ModContainer *container,
                                                   LIBMATTI_BUS_EventPriority phase, LIBMATTI_BUS_Event *event);

#endif //MATTICRAFT_FML_MODCONTAINER_H
