// Port of net.neoforged.fml.classloading.transformation.TransformingClassLoader.

#include "libmatti/net/neoforged/fml/classloading/transformation/TransformingClassLoader.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Java: the anonymous ClassHierarchyRecomputationContext
static void *recomputation_find_loaded_class(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                             const char *className);

static unsigned char *recomputation_up_to_frames(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                                 const char *className, size_t *length);

static void *recomputation_locate_parent_class(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                               const char *className);

unsigned char *LIBMATTI_FML_TransformingClassLoader_BuildTransformedClassNodeFor(
    LIBMATTI_FML_TransformingClassLoader *classLoader, const char *className,
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer, size_t *outLength);

// Java: the Function<ProcessorName, BytecodeProvider> passed to ClassProcessorSet.link
typedef struct
{
    LIBMATTI_NEOFORGESPI_BytecodeProvider provider;
    LIBMATTI_FML_TransformingClassLoader *classLoader;
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer;
} LoaderBytecodeProvider;

static unsigned char *loader_bytecode_provider_get(LIBMATTI_NEOFORGESPI_BytecodeProvider *self, const char *className,
                                                   size_t *length)
{
    LoaderBytecodeProvider *provider = (LoaderBytecodeProvider *) self;
    return LIBMATTI_FML_TransformingClassLoader_BuildTransformedClassNodeFor(provider->classLoader, className,
                                                                           provider->upToTransformer, length);
}

// Java: processorName -> className -> buildTransformedClassNodeFor(className, processorName)
static LIBMATTI_NEOFORGESPI_BytecodeProvider *bytecode_provider_lookup(
    const LIBMATTI_NEOFORGESPI_ProcessorName *name, void *userdata)
{
    LoaderBytecodeProvider *provider = calloc(1, sizeof(LoaderBytecodeProvider));
    provider->provider.getByteCode = loader_bytecode_provider_get;
    provider->classLoader = userdata;
    provider->upToTransformer = name;
    return &provider->provider;
}

// Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, @Nullable String upToTransformer)
static unsigned char *transforming_maybe_transform(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                                   size_t length, const char *name, const char *upToTransformer,
                                                   size_t *outLength)
{
    LIBMATTI_FML_TransformingClassLoader *classLoader = (LIBMATTI_FML_TransformingClassLoader *) self;

    // Java: var upToTransformerName = upToTransformer == null ? null : ProcessorName.parse(upToTransformer);
    LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformerName =
        upToTransformer == NULL ? NULL : LIBMATTI_NEOFORGESPI_ProcessorName_Parse(upToTransformer);

    // Java: classTransformer.transform(bytes, name, upToTransformerName, new ClassHierarchyRecomputationContext() {...})
    unsigned char *result = LIBMATTI_FML_ClassTransformer_Transform(
        classLoader->classTransformer, bytes, length, name, upToTransformerName, &classLoader->recomputationContext,
        outLength);

    LIBMATTI_NEOFORGESPI_ProcessorName_Free(upToTransformerName);
    return result;
}

LIBMATTI_FML_TransformingClassLoader *LIBMATTI_FML_TransformingClassLoader_New(
    LIBMATTI_FML_ClassProcessorSet *classProcessorSet, LIBMATTI_FML_ClassProcessorAuditLog *auditTrail,
    LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount,
    LIBMATTI_JL_ClassLoader *parentClassLoader)
{
    LIBMATTI_FML_TransformingClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_FML_TransformingClassLoader));

    // Java: super("TRANSFORMER", configuration, parentLayers, parentClassLoader)
    LIBMATTI_FML_ModuleClassLoader *base = LIBMATTI_FML_ModuleClassLoader_NewWithParent(
        "TRANSFORMER", configuration, parentLayers, parentLayerCount, parentClassLoader);
    classLoader->base = *base;
    free(base);
    LIBMATTI_FML_ModuleClassLoader_SetMaybeTransformClassBytes(&classLoader->base, transforming_maybe_transform);

    classLoader->classTransformer = LIBMATTI_FML_ClassTransformer_New(classProcessorSet, auditTrail);

    classLoader->recomputationContext.findLoadedClass = recomputation_find_loaded_class;
    classLoader->recomputationContext.upToFrames = recomputation_up_to_frames;
    classLoader->recomputationContext.locateParentClass = recomputation_locate_parent_class;

    // Java: classProcessorSet.link(processorName -> className -> buildTransformedClassNodeFor(className, processorName))
    LIBMATTI_FML_ClassProcessorSet_Link(classProcessorSet, bytecode_provider_lookup, classLoader);
    return classLoader;
}

void LIBMATTI_FML_TransformingClassLoader_Free(LIBMATTI_FML_TransformingClassLoader *classLoader)
{
    if (classLoader == NULL) return;
    LIBMATTI_FML_ClassTransformer_Free(classLoader->classTransformer);
    LIBMATTI_FML_ModuleClassLoader_Free(&classLoader->base);
    free(classLoader);
}

unsigned char *LIBMATTI_FML_TransformingClassLoader_MaybeTransformClassBytes(
    LIBMATTI_FML_TransformingClassLoader *classLoader, const unsigned char *bytes, size_t length, const char *name,
    const char *upToTransformer, size_t *outLength)
{
    return transforming_maybe_transform(&classLoader->base, bytes, length, name, upToTransformer, outLength);
}

void *LIBMATTI_FML_TransformingClassLoader_GetLoadedClass(const LIBMATTI_FML_TransformingClassLoader *classLoader,
                                                          const char *name)
{
    // Java: return findLoadedClass(name); // JVM-backed (external)
    (void) classLoader;
    (void) name;
    return NULL;
}

unsigned char *LIBMATTI_FML_TransformingClassLoader_BuildTransformedClassNodeFor(
    LIBMATTI_FML_TransformingClassLoader *classLoader, const char *className,
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer, size_t *outLength)
{
    // Java: return super.getMaybeTransformedClassBytes(className, upToTransformer.toString());
    char *upToTransformerString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(upToTransformer);
    unsigned char *result = LIBMATTI_FML_ModuleClassLoader_GetMaybeTransformedClassBytes(
        &classLoader->base, className, upToTransformerString, outLength);
    free(upToTransformerString);
    return result;
}

// Java: new ClassHierarchyRecomputationContext() { findLoadedClass / upToFrames / locateParentClass }
static LIBMATTI_FML_TransformingClassLoader *recomputation_owner(LIBMATTI_FML_ClassHierarchyRecomputationContext *self)
{
    return (LIBMATTI_FML_TransformingClassLoader *) ((char *) self -
        offsetof(LIBMATTI_FML_TransformingClassLoader, recomputationContext));
}

static void *recomputation_find_loaded_class(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                             const char *className)
{
    // Java: return TransformingClassLoader.this.getLoadedClass(name);
    return LIBMATTI_FML_TransformingClassLoader_GetLoadedClass(recomputation_owner(self), className);
}

static unsigned char *recomputation_up_to_frames(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                                 const char *className, size_t *length)
{
    // Java: return TransformingClassLoader.this.buildTransformedClassNodeFor(className, ClassProcessorIds.COMPUTING_FRAMES);
    return LIBMATTI_FML_TransformingClassLoader_BuildTransformedClassNodeFor(
        recomputation_owner(self), className, LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES(), length);
}

static void *recomputation_locate_parent_class(LIBMATTI_FML_ClassHierarchyRecomputationContext *self,
                                               const char *className)
{
    // Java: return Class.forName(className, false, TransformingClassLoader.this.getParent());
    LIBMATTI_FML_TransformingClassLoader *classLoader = recomputation_owner(self);
    return LIBMATTI_JL_Class_ForName(className, 0, classLoader->base.base.parent);
}
