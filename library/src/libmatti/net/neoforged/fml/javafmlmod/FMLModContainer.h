// Port of net.neoforged.fml.javafmlmod.FMLModContainer.
// TODO: org.spongepowered.asm.mixin.* (FabricUtil, IMixinInfo, MixinApplyError, InvalidMixinException,
//       MixinTransformerError) - handleMixinError needs the mixin transformer, which is not ported.
// TODO: java.lang.reflect.Constructor / InvocationTargetException - constructMod reflects on the mod
//       class's public constructor; C has neither classes nor reflection, so the mod registers its
//       constructor together with the argument types it declares and constructMod validates them the
//       same way Java validates the constructor's parameter types.
// TODO: java.lang.Module / ModuleLayer.findModule - the C layer is kept by name.

#ifndef MATTICRAFT_FML_JAVAFMLMOD_FMLMODCONTAINER_H
#define MATTICRAFT_FML_JAVAFMLMOD_FMLMODCONTAINER_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_FMLModContainer LIBMATTI_FML_FMLModContainer;

// Port-only: the mod's code as a loaded shared object (net/neoforged/fml/loading/ModLibrary.h).
typedef struct LIBMATTI_FML_ModLibrary LIBMATTI_FML_ModLibrary;

// Java: Map<Class<?>, Object> allowedConstructorArgs = Map.of(IEventBus.class, eventBus,
//         ModContainer.class, this, FMLModContainer.class, this, Dist.class, FMLLoader.getCurrent().getDist())
typedef enum
{
    LIBMATTI_FML_FMLModContainer_ARG_EVENT_BUS,
    LIBMATTI_FML_FMLModContainer_ARG_MOD_CONTAINER,
    LIBMATTI_FML_FMLModContainer_ARG_FML_MOD_CONTAINER,
    LIBMATTI_FML_FMLModContainer_ARG_DIST
} LIBMATTI_FML_FMLModContainer_Arg;

// Java: Object[] constructorArgs
typedef struct
{
    LIBMATTI_BUS_IEventBus *eventBus;
    LIBMATTI_FML_ModContainer *modContainer;
    LIBMATTI_FML_FMLModContainer *fmlModContainer;
    LIBMATTI_DIST_Dist dist;
} LIBMATTI_FML_FMLModContainer_ConstructorArgs;

// Java: one entry of List<Class<?>> modClasses plus its resolved public constructor
typedef struct
{
    char *className;
    // Java: constructor.getParameterTypes()
    LIBMATTI_FML_FMLModContainer_Arg *args;
    size_t argCount;
    // Java: constructor.newInstance(constructorArgs)
    void (*construct)(void *self, LIBMATTI_FML_FMLModContainer *container,
                      const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args);
    void *self;
} LIBMATTI_FML_FMLModContainer_ModClass;

// Java: public class FMLModContainer extends ModContainer
struct LIBMATTI_FML_FMLModContainer
{
    // Java: extends ModContainer
    LIBMATTI_FML_ModContainer base;

    // Java: private final ModFileScanData scanResults
    LIBMATTI_NEOFORGESPI_ModFileScanData *scanResults;
    // Java: private final IEventBus eventBus
    LIBMATTI_BUS_IEventBus *eventBus;
    // Java: private final List<Class<?>> modClasses
    char **entrypoints;
    size_t entrypointCount;
    LIBMATTI_FML_FMLModContainer_ModClass *modClasses;
    size_t modClassCount;
    // Java: private final Module layer - the port keeps the module's name
    char *layer;
    // Java: Class.forName(layer, entrypoint) defines the mod's classes; the port loads the mod file's
    // shared object instead. NULL when the mod file carries no code.
    LIBMATTI_FML_ModLibrary *library;
};

// Java: public FMLModContainer(IModInfo info, List<String> entrypoints, ModFileScanData modFileScanResults, ModuleLayer gameLayer)
// 'gameLayer' is the name of the layer module identified by info.getOwningFile().getFile().getId().
LIBMATTI_FML_FMLModContainer *LIBMATTI_FML_FMLModContainer_New(
    LIBMATTI_NEOFORGESPI_IModInfo *info, const char **entrypoints, size_t entrypointCount,
    LIBMATTI_NEOFORGESPI_ModFileScanData *modFileScanResults, const char *gameLayer);
void LIBMATTI_FML_FMLModContainer_Free(LIBMATTI_FML_FMLModContainer *container);

// Java: Class.forName(layer, entrypoint) + constructor
void LIBMATTI_FML_FMLModContainer_RegisterModClass(
    LIBMATTI_FML_FMLModContainer *container, const char *className,
    const LIBMATTI_FML_FMLModContainer_Arg *args, size_t argCount,
    void (*construct)(void *self, LIBMATTI_FML_FMLModContainer *container,
                      const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args),
    void *self);

// Java: public IEventBus getEventBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_FMLModContainer_GetEventBus(LIBMATTI_FML_FMLModContainer *container);

#endif //MATTICRAFT_FML_JAVAFMLMOD_FMLMODCONTAINER_H
