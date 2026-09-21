// Port of cpw.mods.modlauncher.ClassTransformer.

#ifndef MATTICRAFT_MODLAUNCHER_CLASSTRANSFORMER_H
#define MATTICRAFT_MODLAUNCHER_CLASSTRANSFORMER_H

#include "libmatti/cpw/modlauncher/LaunchPluginHandler.h"
#include "libmatti/cpw/modlauncher/TransformStore.h"
#include "libmatti/cpw/modlauncher/TransformerAuditTrail.h"
#include "libmatti/cpw/modlauncher/TransformingClassLoader.h"

#include <stddef.h>

// Java: public class ClassTransformer
typedef struct LIBMATTI_ML_ClassTransformer
{
    LIBMATTI_ML_TransformStore *transformers;
    LIBMATTI_ML_LaunchPluginHandler *pluginHandler;
    LIBMATTI_ML_TransformingClassLoader *transformingClassLoader;
    LIBMATTI_ML_TransformerAuditTrail *auditTrail;
} LIBMATTI_ML_ClassTransformer;

// Java: ClassTransformer(TransformStore, LaunchPluginHandler, TransformingClassLoader)
LIBMATTI_ML_ClassTransformer *LIBMATTI_ML_ClassTransformer_New(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_TransformingClassLoader *transformingClassLoader);
// Java: ClassTransformer(TransformStore, LaunchPluginHandler, TransformingClassLoader, TransformerAuditTrail)
LIBMATTI_ML_ClassTransformer *LIBMATTI_ML_ClassTransformer_NewWithTrail(
    LIBMATTI_ML_TransformStore *transformStore, LIBMATTI_ML_LaunchPluginHandler *pluginHandler,
    LIBMATTI_ML_TransformingClassLoader *transformingClassLoader, LIBMATTI_ML_TransformerAuditTrail *auditTrail);
void LIBMATTI_ML_ClassTransformer_Free(LIBMATTI_ML_ClassTransformer *transformer);

// Java: byte[] transform(byte[] inputClass, String className, String reason); caller frees
unsigned char *LIBMATTI_ML_ClassTransformer_Transform(LIBMATTI_ML_ClassTransformer *transformer,
                                                      const unsigned char *inputClass, size_t inputLength,
                                                      const char *className, const char *reason, size_t *outLength);

// Java: TransformingClassLoader getTransformingClassLoader()
LIBMATTI_ML_TransformingClassLoader *LIBMATTI_ML_ClassTransformer_GetTransformingClassLoader(
    const LIBMATTI_ML_ClassTransformer *transformer);

#endif //MATTICRAFT_MODLAUNCHER_CLASSTRANSFORMER_H
