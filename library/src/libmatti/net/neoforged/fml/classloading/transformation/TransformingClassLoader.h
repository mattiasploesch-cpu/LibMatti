// Port of net.neoforged.fml.classloading.transformation.TransformingClassLoader.
// It extends net.neoforged.fml.classloading.ModuleClassLoader and overrides
// maybeTransformClassBytes, which the C port expresses through the base class' function pointer hook.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMINGCLASSLOADER_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMINGCLASSLOADER_H

#include "libmatti/net/neoforged/fml/classloading/ModuleClassLoader.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassHierarchyRecomputationContext.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditLog.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorSet.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassTransformer.h"

// Java: public class TransformingClassLoader extends ModuleClassLoader
typedef struct LIBMATTI_FML_TransformingClassLoader
{
    LIBMATTI_FML_ModuleClassLoader base;
    LIBMATTI_FML_ClassTransformer *classTransformer;
    // Java: the anonymous ClassHierarchyRecomputationContext created in maybeTransformClassBytes
    LIBMATTI_FML_ClassHierarchyRecomputationContext recomputationContext;
} LIBMATTI_FML_TransformingClassLoader;

// Java: public TransformingClassLoader(ClassProcessorSet, ClassProcessorAuditLog, Configuration, List<ModuleLayer>, ClassLoader parentClassLoader)
LIBMATTI_FML_TransformingClassLoader *LIBMATTI_FML_TransformingClassLoader_New(
    LIBMATTI_FML_ClassProcessorSet *classProcessorSet, LIBMATTI_FML_ClassProcessorAuditLog *auditTrail,
    LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parentLayers, size_t parentLayerCount,
    LIBMATTI_JL_ClassLoader *parentClassLoader);
void LIBMATTI_FML_TransformingClassLoader_Free(LIBMATTI_FML_TransformingClassLoader *classLoader);

// Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, @Nullable String upToTransformer)
unsigned char *LIBMATTI_FML_TransformingClassLoader_MaybeTransformClassBytes(
    LIBMATTI_FML_TransformingClassLoader *classLoader, const unsigned char *bytes, size_t length, const char *name,
    const char *upToTransformer, size_t *outLength);

// Java: private Class<?> getLoadedClass(String name) - findLoadedClass is JVM-backed, so NULL
void *LIBMATTI_FML_TransformingClassLoader_GetLoadedClass(const LIBMATTI_FML_TransformingClassLoader *classLoader,
                                                          const char *name);

// Java: byte[] buildTransformedClassNodeFor(String className, ProcessorName upToTransformer); NULL = ClassNotFoundException
unsigned char *LIBMATTI_FML_TransformingClassLoader_BuildTransformedClassNodeFor(
    LIBMATTI_FML_TransformingClassLoader *classLoader, const char *className,
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer, size_t *outLength);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_TRANSFORMINGCLASSLOADER_H
