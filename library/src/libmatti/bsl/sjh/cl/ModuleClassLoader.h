// Port of cpw.mods.cl.ModuleClassLoader.

#ifndef MATTICRAFT_MODULECLASSLODER_H
#define MATTICRAFT_MODULECLASSLODER_H

#include "libmatti/bsl/sjh/cl/JarModuleFinder.h"
#include "libmatti/java/lang/module/Configuration.h"

#include <stddef.h>

// external (JVM-backed), opaque: java.lang.module.ModuleLayer
typedef struct LIBMATTI_CL_ModuleLayer LIBMATTI_CL_ModuleLayer;
// external (JVM-backed), opaque: java.lang.ClassLoader
typedef struct LIBMATTI_CL_ClassLoader LIBMATTI_CL_ClassLoader;
// external (JVM-backed), opaque: java.lang.Module
typedef struct LIBMATTI_CL_Module LIBMATTI_CL_Module;
// external (JVM-backed), opaque: java.lang.Package
typedef struct LIBMATTI_CL_Package LIBMATTI_CL_Package;

// Java: Map<String, JarModuleReference> resolvedRoots
typedef struct
{
    char **names;
    LIBMATTI_CL_JarModuleReference **references;
    size_t count;
} LIBMATTI_CL_ResolvedRoots;

// Java: Map<String, ResolvedModule> packageLookup (module name per package)
typedef struct
{
    char **packages;
    char **moduleNames;
    size_t count;
} LIBMATTI_CL_PackageLookup;

// Java: Map<String, ClassLoader> parentLoaders
typedef struct
{
    char **packages;
    LIBMATTI_CL_ClassLoader **classLoaders;
    size_t count;
} LIBMATTI_CL_ParentLoaders;

// Java: class ModuleClassLoader extends ClassLoader
typedef struct LIBMATTI_CL_ModuleClassLoader
{
    char *name;
    LIBMATTI_JL_Configuration *configuration;
    LIBMATTI_CL_ResolvedRoots resolvedRoots;
    LIBMATTI_CL_PackageLookup packageLookup;
    LIBMATTI_CL_ParentLoaders parentLoaders;
    // Java: @Nullable ClassLoader fallbackClassLoader (NULL until set)
    LIBMATTI_CL_ClassLoader *fallbackClassLoader;
    // Java: protected byte[] maybeTransformClassBytes(byte[] bytes, String name, String context)
    // Java overrides this in TransformingClassLoader; the C port uses a function pointer.
    unsigned char *(*maybeTransformClassBytes)(struct LIBMATTI_CL_ModuleClassLoader *self,
                                               const unsigned char *bytes, size_t length, const char *name,
                                               const char *context, size_t *outLength);
} LIBMATTI_CL_ModuleClassLoader;

// Java: ModuleClassLoader(String name, Configuration, List<ModuleLayer>)
// The parentLayers list only feeds JVM-external logic (layer.findLoader,
// ModuleLayer.bindToLoader) and is therefore omitted here.
LIBMATTI_CL_ModuleClassLoader *LIBMATTI_CL_ModuleClassLoader_New(const char *name, LIBMATTI_JL_Configuration *configuration);
void LIBMATTI_CL_ModuleClassLoader_Free(LIBMATTI_CL_ModuleClassLoader *classLoader);

// Java: BiFunction<ModuleReader, ModuleReference, T> lookup
typedef void *(*LIBMATTI_CL_Lookup)(LIBMATTI_CL_JarModuleReader *reader, LIBMATTI_CL_JarModuleReference *ref, void *userData);

// Java: Class<?> loadClass(String name, boolean resolve); NULL = ClassNotFoundException
void *LIBMATTI_CL_ModuleClassLoader_LoadClass(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name, int resolve);
// Java: Class<?> findClass(String name)
void *LIBMATTI_CL_ModuleClassLoader_FindClass(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name);
// Java: Class<?> findClass(String moduleName, String name)
void *LIBMATTI_CL_ModuleClassLoader_FindClassInModule(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName, const char *name);
// Java: String classNameToModuleName(String name)
char *LIBMATTI_CL_ModuleClassLoader_ClassNameToModuleName(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name);

// Java: URL getResource(String name)
LIBMATTI_JN_URI *LIBMATTI_CL_ModuleClassLoader_GetResource(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name);
// Java: List<URL> findResourceList(String name); caller frees the array
LIBMATTI_JN_URI **LIBMATTI_CL_ModuleClassLoader_FindResourceList(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name, size_t *count);
// Java: URL findResource(String moduleName, String name)
LIBMATTI_JN_URI *LIBMATTI_CL_ModuleClassLoader_FindResource(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName, const char *name);

// Java: byte[] getClassBytes(ModuleReader, ModuleReference, String name); caller frees
unsigned char *LIBMATTI_CL_ModuleClassLoader_GetClassBytes(LIBMATTI_CL_JarModuleReader *reader, const char *name, size_t *outLength);

// Java: byte[] maybeTransformClassBytes(...) - identity
unsigned char *LIBMATTI_CL_ModuleClassLoader_MaybeTransformClassBytes(const unsigned char *bytes, size_t length, size_t *outLength);
// Java: the subclass override
void LIBMATTI_CL_ModuleClassLoader_SetMaybeTransformClassBytes(
    LIBMATTI_CL_ModuleClassLoader *classLoader,
    unsigned char *(*maybeTransformClassBytes)(LIBMATTI_CL_ModuleClassLoader *self, const unsigned char *bytes,
                                               size_t length, const char *name, const char *context,
                                               size_t *outLength));

// Java: <T> T loadFromModule(String moduleName, BiFunction lookup)
void *LIBMATTI_CL_ModuleClassLoader_LoadFromModule(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName,
                                                   LIBMATTI_CL_Lookup lookup, void *userData);

// Java: byte[] getMaybeTransformedClassBytes(String name, String context); NULL = ClassNotFoundException
unsigned char *LIBMATTI_CL_ModuleClassLoader_GetMaybeTransformedClassBytes(LIBMATTI_CL_ModuleClassLoader *classLoader,
                                                                           const char *name, const char *context,
                                                                           size_t *outLength);

// Java: void setFallbackClassLoader(ClassLoader)
void LIBMATTI_CL_ModuleClassLoader_SetFallbackClassLoader(LIBMATTI_CL_ModuleClassLoader *classLoader, LIBMATTI_CL_ClassLoader *fallbackClassLoader);

#endif //MATTICRAFT_MODULECLASSLODER_H