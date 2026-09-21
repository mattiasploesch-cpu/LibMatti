// Port of net.neoforged.fml.classloading.ModuleClassLoader.
// This classloader implements child-first classloading for any module that is defined locally.
// Note: it is a different Java class from cpw.mods.cl.ModuleClassLoader (bsl/sjh/cl).

#ifndef MATTICRAFT_FML_CLASSLOADING_MODULECLASSLODER_H
#define MATTICRAFT_FML_CLASSLOADING_MODULECLASSLODER_H

#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/java/lang/module/Configuration.h"
#include "libmatti/java/lang/module/ModuleReference.h"
#include "libmatti/net/neoforged/fml/classloading/JarContentsModuleReference.h"

#include <stddef.h>

// external (JVM-backed), opaque: java.lang.module.ModuleLayer
typedef struct LIBMATTI_JL_ModuleLayer LIBMATTI_JL_ModuleLayer;

// Java: private static final class ModuleInfo implements AutoCloseable
typedef struct LIBMATTI_FML_ModuleInfo LIBMATTI_FML_ModuleInfo;

struct LIBMATTI_FML_ModuleInfo
{
    char *name;
    LIBMATTI_JL_ModuleReference *moduleReference;
    // Java: private final ReentrantLock lock - the ported launcher loads on one thread
    // Java: private volatile ModuleReader cachedReader
    LIBMATTI_FML_JarContentsModuleReader *cachedReader;
    // Java: private volatile boolean closed
    int closed;
    // Java: private final ProtectionDomain protectionDomain
    // TODO: java.security.ProtectionDomain (CodeSource, Permissions, AllPermission) is JVM-backed
    void *protectionDomain;
};

// Java: public ModuleClassLoader(String name, Configuration configuration, List<ModuleLayer> parentLayers)
typedef struct LIBMATTI_FML_ModuleClassLoader LIBMATTI_FML_ModuleClassLoader;

struct LIBMATTI_FML_ModuleClassLoader
{
    // Java: public class ModuleClassLoader extends ClassLoader
    LIBMATTI_JL_ClassLoader base;

    // Java: private final Map<String, ModuleInfo> moduleInfoCache
    LIBMATTI_FML_ModuleInfo **moduleInfos;
    size_t moduleInfoCount;
    // Java: private final Map<String, ModuleInfo> packageLookup
    char **lookupPackages;
    LIBMATTI_FML_ModuleInfo **lookupModules;
    size_t lookupCount;
    // Java: private final Map<String, ClassLoader> parentLoaders
    char **parentLoaderPackages;
    LIBMATTI_JL_ClassLoader **parentLoaders;
    size_t parentLoaderCount;
    // Java: private final Configuration configuration
    LIBMATTI_JL_Configuration *configuration;
    // Java: private ClassLoader fallbackClassLoader
    LIBMATTI_JL_ClassLoader *fallbackClassLoader;
    // Java: private volatile boolean closed = false
    int closed;

    // Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, @Nullable String context)
    // Java overrides this in TransformingClassLoader; the C port uses a function pointer.
    unsigned char *(*maybeTransformClassBytes)(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                               size_t length, const char *name, const char *context,
                                               size_t *outLength);
};

// Java: public ModuleClassLoader(String name, Configuration configuration, List<ModuleLayer> parentLayers)
LIBMATTI_FML_ModuleClassLoader *LIBMATTI_FML_ModuleClassLoader_New(const char *name,
                                                                  LIBMATTI_JL_Configuration *configuration,
                                                                  LIBMATTI_JL_ModuleLayer **parentLayers,
                                                                  size_t parentLayerCount);
// Java: public ModuleClassLoader(String name, Configuration configuration, List<ModuleLayer> parentLayers, @Nullable ClassLoader parentLoader)
LIBMATTI_FML_ModuleClassLoader *LIBMATTI_FML_ModuleClassLoader_NewWithParent(
    const char *name, LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parentLayers,
    size_t parentLayerCount, LIBMATTI_JL_ClassLoader *parentLoader);
void LIBMATTI_FML_ModuleClassLoader_Free(LIBMATTI_FML_ModuleClassLoader *classLoader);
// Java: @Override public void close() throws IOException
void LIBMATTI_FML_ModuleClassLoader_Close(LIBMATTI_FML_ModuleClassLoader *classLoader);

// Java: ModuleInfo constructor and reader handling
LIBMATTI_FML_ModuleInfo *LIBMATTI_FML_ModuleInfo_New(LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                     LIBMATTI_JL_ModuleReference *moduleReference);
// Java: ModuleReader getReader() throws IOException - NULL on failure
LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_ModuleInfo_GetReader(LIBMATTI_FML_ModuleInfo *moduleInfo);
// Java: public void close()
void LIBMATTI_FML_ModuleInfo_Close(LIBMATTI_FML_ModuleInfo *moduleInfo);

// Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, @Nullable String context) - identity
unsigned char *LIBMATTI_FML_ModuleClassLoader_MaybeTransformClassBytes(const unsigned char *bytes, size_t length,
                                                                      size_t *outLength);
// Java: the subclass override
void LIBMATTI_FML_ModuleClassLoader_SetMaybeTransformClassBytes(
    LIBMATTI_FML_ModuleClassLoader *classLoader,
    unsigned char *(*maybeTransformClassBytes)(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                               size_t length, const char *name, const char *context,
                                               size_t *outLength));

// Java: protected Class<?> loadClass(String name, boolean resolve) - NULL = ClassNotFoundException
void *LIBMATTI_FML_ModuleClassLoader_LoadClass(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *name,
                                              int resolve);
// Java: public URL getResource(String name)
LIBMATTI_JN_URI *LIBMATTI_FML_ModuleClassLoader_GetResource(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                            const char *name);
// Java: protected URL findResource(String moduleName, String name)
LIBMATTI_JN_URI *LIBMATTI_FML_ModuleClassLoader_FindResourceForModule(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                                     const char *moduleName, const char *name);
// Java: public Enumeration<URL> getResources(String name); caller frees the array
LIBMATTI_JN_URI **LIBMATTI_FML_ModuleClassLoader_GetResources(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                             const char *name, size_t *count);
// Java: protected Enumeration<URL> findResources(String name); caller frees the array
LIBMATTI_JN_URI **LIBMATTI_FML_ModuleClassLoader_FindResources(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                              const char *name, size_t *count);
// Java: protected Class<?> findClass(String moduleName, String name)
void *LIBMATTI_FML_ModuleClassLoader_FindClassInModule(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                       const char *moduleName, const char *name);
// Java: protected Class<?> findClass(String name) throws ClassNotFoundException
void *LIBMATTI_FML_ModuleClassLoader_FindClass(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *name);
// Java: protected byte[] getMaybeTransformedClassBytes(String name, String context); NULL = ClassNotFoundException
unsigned char *LIBMATTI_FML_ModuleClassLoader_GetMaybeTransformedClassBytes(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                                          const char *name, const char *context,
                                                                          size_t *outLength);
// Java: public void setFallbackClassLoader(ClassLoader fallbackClassLoader)
void LIBMATTI_FML_ModuleClassLoader_SetFallbackClassLoader(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                          LIBMATTI_JL_ClassLoader *fallbackClassLoader);
// Java: public Configuration getConfiguration()
LIBMATTI_JL_Configuration *LIBMATTI_FML_ModuleClassLoader_GetConfiguration(
    const LIBMATTI_FML_ModuleClassLoader *classLoader);

#endif //MATTICRAFT_FML_CLASSLOADING_MODULECLASSLODER_H
