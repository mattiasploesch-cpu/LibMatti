// Port of cpw.mods.modlauncher.TransformingClassLoader.

#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"

#include "libmatti/cpw/modlauncher/ClassTransformer.h"
#include "libmatti/cpw/modlauncher/TransformerAuditTrail.h"

#include <stdlib.h>
#include <string.h>

// Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, String context)
static unsigned char *transforming_maybe_transform(LIBMATTI_CL_ModuleClassLoader *self, const unsigned char *bytes,
                                                   size_t length, const char *name, const char *context,
                                                   size_t *outLength)
{
    LIBMATTI_ML_TransformingClassLoader *classLoader = (LIBMATTI_ML_TransformingClassLoader *)self;

    // Java: return classTransformer.transform(bytes, name, context != null ? context : CLASSLOADING_REASON)
    return LIBMATTI_ML_ClassTransformer_Transform(classLoader->classTransformer, bytes, length, name,
                                                 context != NULL
                                                     ? context
                                                     : LIBMATTI_MLA_ITransformerActivity_CLASSLOADING_REASON,
                                                 outLength);
}

LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_MLA_IModuleLayerManager *moduleLayerHandler)
{
    LIBMATTI_ML_TransformingClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_ML_TransformingClassLoader));

    // Java: super("TRANSFORMER", moduleLayerHandler.getLayer(Layer.GAME).orElseThrow().configuration(),
    //            List.of(moduleLayerHandler.getLayer(Layer.SERVICE).orElseThrow()))
    LIBMATTI_JL_ModuleLayer *gameLayer = LIBMATTI_MLA_IModuleLayerManager_GetLayer(moduleLayerHandler,
                                                                                   LIBMATTI_MLA_LAYER_GAME);
    LIBMATTI_JL_Configuration *configuration = gameLayer != NULL ? LIBMATTI_JL_ModuleLayer_Configuration(gameLayer)
                                                                 : NULL;

    // Java: extends ModuleClassLoader - the base state is embedded and its heap shell released
    LIBMATTI_CL_ModuleClassLoader *base = LIBMATTI_CL_ModuleClassLoader_New("TRANSFORMER", configuration);
    classLoader->base = *base;
    free(base);
    LIBMATTI_CL_ModuleClassLoader_SetMaybeTransformClassBytes(&classLoader->base, transforming_maybe_transform);

    classLoader->classTransformer = LIBMATTI_ML_ClassTransformer_New(transformStore, pluginHandler, classLoader);
    return classLoader;
}

LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_NewWithEnvironment(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_JL_Configuration *configuration,
    LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount)
{
    return LIBMATTI_ML_TransformingClassLoader_NewWithParent(transformStore, pluginHandler, environment, configuration,
                                                             parentLayers, parentLayerCount, NULL);
}

// Java: the environment.computePropertyIfAbsent(AUDITTRAIL, v -> tat) value function
static void *supply_audit_trail(LIBMATTI_MLA_Key *key, void *userdata)
{
    (void)key;
    return userdata;
}

LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_TransformingClassLoader_NewWithParent(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_Environment *environment, LIBMATTI_JL_Configuration *configuration,
    LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount, void *parentClassLoader)
{
    (void)parentLayers;
    (void)parentLayerCount;
    (void)parentClassLoader;

    LIBMATTI_ML_TransformingClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_ML_TransformingClassLoader));
    LIBMATTI_CL_ModuleClassLoader *base = LIBMATTI_CL_ModuleClassLoader_New("TRANSFORMER", configuration);
    classLoader->base = *base;
    free(base);
    LIBMATTI_CL_ModuleClassLoader_SetMaybeTransformClassBytes(&classLoader->base, transforming_maybe_transform);

    // Java: TransformerAuditTrail tat = new TransformerAuditTrail();
    //       environment.computePropertyIfAbsent(IEnvironment.Keys.AUDITTRAIL.get(), v -> tat);
    LIBMATTI_ML_TransformerAuditTrail *tat = LIBMATTI_ML_TransformerAuditTrail_New();
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(environment, LIBMATTI_MLA_IEnvironment_Keys_AuditTrail(),
                                                    supply_audit_trail, tat);

    classLoader->classTransformer = LIBMATTI_ML_ClassTransformer_NewWithTrail(transformStore, pluginHandler,
                                                                              classLoader, tat);
    return classLoader;
}

void LIBMATTI_ML_TransformingClassLoader_Free(LIBMATTI_ML_TransformingClassLoader *classLoader)
{
    if (classLoader == NULL) return;
    LIBMATTI_ML_ClassTransformer_Free(classLoader->classTransformer);
    free(classLoader);
}

unsigned char *LIBMATTI_ML_TransformingClassLoader_MaybeTransformClassBytes(
    LIBMATTI_ML_TransformingClassLoader *classLoader, const unsigned char *bytes, size_t length, const char *name,
    const char *context, size_t *outLength)
{
    return transforming_maybe_transform(&classLoader->base, bytes, length, name, context, outLength);
}

void *LIBMATTI_ML_TransformingClassLoader_GetLoadedClass(const LIBMATTI_ML_TransformingClassLoader *classLoader,
                                                         const char *name)
{
    // Java: return findLoadedClass(name); // JVM-backed (external)
    (void)classLoader;
    (void)name;
    return NULL;
}

unsigned char *LIBMATTI_ML_TransformingClassLoader_BuildTransformedClassNodeFor(
    LIBMATTI_ML_TransformingClassLoader *classLoader, const char *className, const char *reason, size_t *outLength)
{
    // Java: return super.getMaybeTransformedClassBytes(className, reason)
    return LIBMATTI_CL_ModuleClassLoader_GetMaybeTransformedClassBytes(&classLoader->base, className, reason,
                                                                     outLength);
}
