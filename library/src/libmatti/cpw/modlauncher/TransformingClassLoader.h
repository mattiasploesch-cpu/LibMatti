// Port of cpw.mods.modlauncher.TransformingClassLoader.
// It extends cpw.mods.cl.ModuleClassLoader and overrides maybeTransformClassBytes,
// which the C port expresses through the base class' function pointer hook.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMINGCLASSLOADER_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMINGCLASSLOADER_H

#include "libmatti/bsl/sjh/cl/ModuleClassLoader.h"
#include "libmatti/cpw/modlauncher/Environment.h"
#include "libmatti/cpw/modlauncher/api/IModuleLayerManager.h"
#include "libmatti/java/lang/ModuleLayer.h"
#include "libmatti/java/lang/module/Configuration.h"

#include <stddef.h>

typedef struct LIBMATTI_ML_ClassTransformer LIBMATTI_ML_ClassTransformer;
typedef struct LIBMATTI_ML_LaunchPluginHandler LIBMATTI_ML_LaunchPluginHandler;
typedef struct LIBMATTI_ML_TransformStore LIBMATTI_ML_TransformStore;

// Java: public class TransformingClassLoader extends ModuleClassLoader
typedef struct LIBMATTI_ML_TransformingClassLoader
{
    LIBMATTI_CL_ModuleClassLoader base;
    LIBMATTI_ML_ClassTransformer *classTransformer;
} LIBMATTI_ML_TransformingClassLoader;

// Java: public TransformingClassLoader(TransformStore, LaunchPluginHandler, IModuleLayerManager)
LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_MLA_IModuleLayerManager *moduleLayerHandler);
// Java: public TransformingClassLoader(TransformStore, LaunchPluginHandler, Environment, Configuration, List<ModuleLayer>)
LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_NewWithEnvironment(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_JL_Configuration *configuration,
    LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount);
// Java: ...(... ClassLoader parentClassLoader)
LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_NewWithParent(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_JL_Configuration *configuration,
    LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount, void *parentClassLoader);
void LIBMATTI_ML_TransformingClassLoader_Free(LIBMATTI_ML_TransformingClassLoader *classLoader);

// Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, String context); caller frees
unsigned char *LIBMATTI_ML_TransformingClassLoader_MaybeTransformClassBytes(
    LIBMATTI_ML_TransformingClassLoader *classLoader, const unsigned char *bytes, size_t length, const char *name,
    const char *context, size_t *outLength);

// Java: public Class<?> getLoadedClass(String name) - findLoadedClass is JVM-backed, so NULL
void *LIBMATTI_ML_TransformingClassLoader_GetLoadedClass(const LIBMATTI_ML_TransformingClassLoader *classLoader,
                                                         const char *name);

// Java: byte[] buildTransformedClassNodeFor(String className, String reason); NULL = ClassNotFoundException
unsigned char *LIBMATTI_ML_TransformingClassLoader_BuildTransformedClassNodeFor(
    LIBMATTI_ML_TransformingClassLoader *classLoader, const char *className, const char *reason, size_t *outLength);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMINGCLASSLOADER_H
