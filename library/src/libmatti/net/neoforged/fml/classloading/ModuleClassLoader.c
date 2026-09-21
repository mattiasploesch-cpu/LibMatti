// Port of net.neoforged.fml.classloading.ModuleClassLoader.

#include "libmatti/net/neoforged/fml/classloading/ModuleClassLoader.h"

#include <stdlib.h>
#include <string.h>

// Java: private static void bindToLayer(ModuleClassLoader classLoader, ModuleLayer layer)
// ModuleLayer.bindToLoader is JVM-internal (reached through MethodHandles IMPL_LOOKUP), no C equivalent.

LIBMATTI_FML_ModuleInfo *LIBMATTI_FML_ModuleInfo_New(LIBMATTI_JL_ClassLoader *classLoader, const char *name,
                                                     LIBMATTI_JL_ModuleReference *moduleReference)
{
    LIBMATTI_FML_ModuleInfo *moduleInfo = calloc(1, sizeof(LIBMATTI_FML_ModuleInfo));
    moduleInfo->name = strdup(name);
    moduleInfo->moduleReference = moduleReference;
    // Java: var codeSource = new CodeSource(toURL(moduleReference.location()), (CodeSigner[]) null);
    //       var perms = new Permissions(); perms.add(new AllPermission());
    //       this.protectionDomain = new ProtectionDomain(codeSource, perms, classLoader, null);
    // TODO: java.security.CodeSource / Permissions / ProtectionDomain are JVM-backed
    (void) classLoader;
    return moduleInfo;
}

LIBMATTI_FML_JarContentsModuleReader *LIBMATTI_FML_ModuleInfo_GetReader(LIBMATTI_FML_ModuleInfo *moduleInfo)
{
    // Java: if (closed) throw new IOException("Module " + name + " has been closed");
    if (moduleInfo->closed) return NULL;
    // Java: ModuleReader reader = cachedReader; if (reader != null) return reader;
    if (moduleInfo->cachedReader != NULL) return moduleInfo->cachedReader;

    // Java: reader = moduleReference.open(); cachedReader = reader; return reader;
    moduleInfo->cachedReader = LIBMATTI_FML_JarContentsModuleReference_Open(
        (const LIBMATTI_FML_JarContentsModuleReference *) moduleInfo->moduleReference);
    return moduleInfo->cachedReader;
}

void LIBMATTI_FML_ModuleInfo_Close(LIBMATTI_FML_ModuleInfo *moduleInfo)
{
    // Java: if (!closed) { closed = true; if (cachedReader != null) { cachedReader.close(); cachedReader = null; } }
    if (moduleInfo->closed) return;
    moduleInfo->closed = 1;
    if (moduleInfo->cachedReader != NULL)
    {
        LIBMATTI_FML_JarContentsModuleReader_Close(moduleInfo->cachedReader);
        free(moduleInfo->cachedReader);
        moduleInfo->cachedReader = NULL;
    }
}

static LIBMATTI_FML_ModuleInfo *module_info_for(const LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                const char *moduleName)
{
    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
    {
        if (strcmp(classLoader->moduleInfos[i]->name, moduleName) == 0) return classLoader->moduleInfos[i];
    }
    return NULL;
}

static LIBMATTI_FML_ModuleInfo *package_module(const LIBMATTI_FML_ModuleClassLoader *classLoader,
                                               const char *packageName)
{
    for (size_t i = 0; i < classLoader->lookupCount; i++)
    {
        if (strcmp(classLoader->lookupPackages[i], packageName) == 0) return classLoader->lookupModules[i];
    }
    return NULL;
}

static void parent_loaders_put(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *packageName,
                               LIBMATTI_JL_ClassLoader *loader)
{
    for (size_t i = 0; i < classLoader->parentLoaderCount; i++)
    {
        if (strcmp(classLoader->parentLoaderPackages[i], packageName) == 0)
        {
            classLoader->parentLoaders[i] = loader;
            return;
        }
    }
    classLoader->parentLoaderPackages = realloc(classLoader->parentLoaderPackages,
                                                sizeof(*classLoader->parentLoaderPackages) * (classLoader->parentLoaderCount + 1));
    classLoader->parentLoaders = realloc(classLoader->parentLoaders,
                                         sizeof(*classLoader->parentLoaders) * (classLoader->parentLoaderCount + 1));
    classLoader->parentLoaderPackages[classLoader->parentLoaderCount] = strdup(packageName);
    classLoader->parentLoaders[classLoader->parentLoaderCount] = loader;
    classLoader->parentLoaderCount++;
}

static LIBMATTI_JL_ClassLoader *parent_loader_for(const LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                  const char *packageName)
{
    for (size_t i = 0; i < classLoader->parentLoaderCount; i++)
    {
        if (strcmp(classLoader->parentLoaderPackages[i], packageName) == 0) return classLoader->parentLoaders[i];
    }
    return classLoader->fallbackClassLoader;
}

static unsigned char *default_maybe_transform(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                              size_t length, const char *name, const char *context,
                                              size_t *outLength);

LIBMATTI_FML_ModuleClassLoader *LIBMATTI_FML_ModuleClassLoader_New(const char *name,
                                                                  LIBMATTI_JL_Configuration *configuration,
                                                                  LIBMATTI_JL_ModuleLayer **parentLayers,
                                                                  size_t parentLayerCount)
{
    return LIBMATTI_FML_ModuleClassLoader_NewWithParent(name, configuration, parentLayers, parentLayerCount, NULL);
}

LIBMATTI_FML_ModuleClassLoader *LIBMATTI_FML_ModuleClassLoader_NewWithParent(
    const char *name, LIBMATTI_JL_Configuration *configuration, LIBMATTI_JL_ModuleLayer **parentLayers,
    size_t parentLayerCount, LIBMATTI_JL_ClassLoader *parentLoader)
{
    LIBMATTI_FML_ModuleClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_FML_ModuleClassLoader));
    // Java: super(name, parentLoader)
    classLoader->base.name = strdup(name);
    classLoader->base.parent = parentLoader;
    classLoader->maybeTransformClassBytes = default_maybe_transform;
    // Java: this.fallbackClassLoader = Objects.requireNonNullElse(parentLoader, ClassLoader.getPlatformClassLoader());
    // getPlatformClassLoader() is JVM-backed (external); NULL means "no fallback" (ClassNotFoundException)
    classLoader->fallbackClassLoader = parentLoader;
    classLoader->configuration = configuration;

    // Java: for (var m : configuration.modules()) { moduleInfoCache.put(moduleName, new ModuleInfo(this, moduleName, m.reference())); }
    size_t moduleCount = 0;
    LIBMATTI_JL_ResolvedModule **modules = LIBMATTI_JL_Configuration_Modules(configuration, &moduleCount);
    classLoader->moduleInfos = calloc(moduleCount > 0 ? moduleCount : 1, sizeof(*classLoader->moduleInfos));
    for (size_t i = 0; i < moduleCount; i++)
    {
        const char *moduleName = LIBMATTI_JL_ResolvedModule_Name(modules[i]);
        classLoader->moduleInfos[classLoader->moduleInfoCount++] = LIBMATTI_FML_ModuleInfo_New(
            &classLoader->base, moduleName, LIBMATTI_JL_ResolvedModule_Reference(modules[i]));
    }

    // Java: for (var moduleInfo : moduleInfoCache.values()) for (var pk : descriptor.packages()) packageLookup.put(pk, moduleInfo)
    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
    {
        LIBMATTI_FML_ModuleInfo *moduleInfo = classLoader->moduleInfos[i];
        size_t packageCount = 0;
        char **packages = LIBMATTI_JL_ModuleDescriptor_Packages(
            LIBMATTI_JL_ModuleReference_Descriptor(moduleInfo->moduleReference), &packageCount);
        classLoader->lookupPackages = realloc(classLoader->lookupPackages,
                                              sizeof(*classLoader->lookupPackages) * (classLoader->lookupCount + packageCount));
        classLoader->lookupModules = realloc(classLoader->lookupModules,
                                             sizeof(*classLoader->lookupModules) * (classLoader->lookupCount + packageCount));
        for (size_t p = 0; p < packageCount; p++)
        {
            classLoader->lookupPackages[classLoader->lookupCount] = strdup(packages[p]);
            classLoader->lookupModules[classLoader->lookupCount] = moduleInfo;
            classLoader->lookupCount++;
        }
    }

    // Java: for (var rm : configuration.modules()) for (var other : rm.reads()) {
    //          ClassLoader cl = classLoaderMap.computeIfAbsent(other, findClassLoader);
    //          ... descriptor.packages() / descriptor.exports() -> parentLoaders.put(pn, cl); }
    // findClassLoader: moduleInfoCache.containsKey(k.name()) ? this : parentLayers.findLoader / platform loader
    char **processedAutomatic = NULL;
    size_t processedCount = 0;
    for (size_t i = 0; i < moduleCount; i++)
    {
        LIBMATTI_JL_ResolvedModule *rm = modules[i];
        size_t readCount = 0;
        LIBMATTI_JL_ResolvedModule **reads = LIBMATTI_JL_ResolvedModule_Reads(rm, &readCount);
        for (size_t r = 0; r < readCount; r++)
        {
            LIBMATTI_JL_ResolvedModule *other = reads[r];
            const char *otherName = LIBMATTI_JL_ResolvedModule_Name(other);
            LIBMATTI_JL_ClassLoader *cl = module_info_for(classLoader, otherName) != NULL
                                              ? &classLoader->base
                                              : NULL;
            LIBMATTI_JL_ModuleDescriptor *descriptor =
                LIBMATTI_JL_ModuleReference_Descriptor(LIBMATTI_JL_ResolvedModule_Reference(other));

            if (LIBMATTI_JL_ModuleDescriptor_IsAutomatic(descriptor))
            {
                // Java: processedAutomaticDescriptors.add(descriptor) - one pass per automatic module
                int processed = 0;
                for (size_t p = 0; p < processedCount; p++)
                {
                    if (strcmp(processedAutomatic[p], otherName) == 0)
                    {
                        processed = 1;
                        break;
                    }
                }
                if (processed) continue;

                processedAutomatic = realloc(processedAutomatic, sizeof(*processedAutomatic) * (processedCount + 1));
                processedAutomatic[processedCount++] = strdup(otherName);

                size_t packageCount = 0;
                char **packages = LIBMATTI_JL_ModuleDescriptor_Packages(descriptor, &packageCount);
                for (size_t p = 0; p < packageCount; p++) parent_loaders_put(classLoader, packages[p], cl);
            }
            else
            {
                // Java: descriptor.exports().filter(e -> !e.isQualified() || (e.isQualified() && other.configuration() == configuration && e.targets().contains(rm.name())))
                size_t exportCount = 0;
                LIBMATTI_JL_ModuleDescriptor_Exports **exports = LIBMATTI_JL_ModuleDescriptor_GetExports(descriptor,
                                                                                                       &exportCount);
                for (size_t e = 0; e < exportCount; e++)
                {
                    LIBMATTI_JL_ModuleDescriptor_Exports *exportEntry = exports[e];
                    int isQualified = exportEntry->targetCount > 0;
                    int include = !isQualified;
                    if (isQualified)
                    {
                        int targetMatch = 0;
                        for (size_t t = 0; t < exportEntry->targetCount; t++)
                        {
                            if (strcmp(exportEntry->targets[t], LIBMATTI_JL_ResolvedModule_Name(rm)) == 0)
                            {
                                targetMatch = 1;
                                break;
                            }
                        }
                        include = LIBMATTI_JL_ResolvedModule_Configuration(other) == configuration && targetMatch;
                    }
                    if (include) parent_loaders_put(classLoader, exportEntry->source, cl);
                }
            }
        }
    }
    for (size_t i = 0; i < processedCount; i++) free(processedAutomatic[i]);
    free(processedAutomatic);

    // Java: parentLayers.forEach(p -> forLayerAndParents(p, visitedLayers, l -> bindToLayer(this, l)));
    // bindToLoader is JVM-internal, no C equivalent.
    (void) parentLayers;
    (void) parentLayerCount;
    return classLoader;
}

void LIBMATTI_FML_ModuleClassLoader_Free(LIBMATTI_FML_ModuleClassLoader *classLoader)
{
    if (classLoader == NULL) return;
    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
    {
        LIBMATTI_FML_ModuleInfo_Close(classLoader->moduleInfos[i]);
        free(classLoader->moduleInfos[i]->name);
        free(classLoader->moduleInfos[i]);
    }
    free(classLoader->moduleInfos);
    for (size_t i = 0; i < classLoader->lookupCount; i++) free(classLoader->lookupPackages[i]);
    free(classLoader->lookupPackages);
    free(classLoader->lookupModules);
    for (size_t i = 0; i < classLoader->parentLoaderCount; i++) free(classLoader->parentLoaderPackages[i]);
    free(classLoader->parentLoaderPackages);
    free(classLoader->parentLoaders);
    free(classLoader->base.name);
    free(classLoader);
}

void LIBMATTI_FML_ModuleClassLoader_Close(LIBMATTI_FML_ModuleClassLoader *classLoader)
{
    // Java: if (closed) return; closed = true; for (ModuleInfo moduleInfo : moduleInfoCache.values()) moduleInfo.close();
    //       moduleInfoCache.clear();
    if (classLoader->closed) return;
    classLoader->closed = 1;

    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
        LIBMATTI_FML_ModuleInfo_Close(classLoader->moduleInfos[i]);

    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
    {
        free(classLoader->moduleInfos[i]->name);
        free(classLoader->moduleInfos[i]);
    }
    classLoader->moduleInfoCount = 0;
}

// Java: private URL readerToURL(ModuleInfo moduleInfo, String name) throws IOException
static LIBMATTI_JN_URI *reader_to_url(LIBMATTI_FML_ModuleInfo *moduleInfo, const char *name)
{
    LIBMATTI_FML_JarContentsModuleReader *reader = LIBMATTI_FML_ModuleInfo_GetReader(moduleInfo);
    if (reader == NULL) return NULL;
    return LIBMATTI_FML_JarContentsModuleReader_Find(reader, name);
}

// Java: private static byte[] getClassBytes(ModuleInfo moduleInfo, String name) throws IOException
static unsigned char *get_class_bytes(LIBMATTI_FML_ModuleInfo *moduleInfo, const char *name, size_t *outLength)
{
    // Java: var cname = name.replace('.', '/') + ".class";
    size_t nameLength = strlen(name);
    char *cname = malloc(nameLength + 7);
    for (size_t i = 0; i < nameLength; i++) cname[i] = name[i] == '.' ? '/' : name[i];
    strcpy(cname + nameLength, ".class");

    LIBMATTI_FML_JarContentsModuleReader *reader = LIBMATTI_FML_ModuleInfo_GetReader(moduleInfo);
    unsigned char *bytes = reader != NULL
                               ? LIBMATTI_FML_JarContentsModuleReader_OpenFile(reader, cname, outLength)
                               : NULL;
    free(cname);
    if (bytes == NULL)
    {
        // Java: return new byte[0];
        *outLength = 0;
        return NULL;
    }
    return bytes;
}

// Java: @Nullable private Class<?> readerToClass(ModuleInfo moduleInfo, String name) throws ClassNotFoundException
static void *reader_to_class(LIBMATTI_FML_ModuleClassLoader *classLoader, LIBMATTI_FML_ModuleInfo *moduleInfo,
                             const char *name)
{
    size_t length = 0;
    unsigned char *bytes = get_class_bytes(moduleInfo, name, &length);
    size_t transformedLength = 0;
    unsigned char *transformed = classLoader->maybeTransformClassBytes(classLoader, bytes, length, name, NULL,
                                                                      &transformedLength);
    free(bytes);
    if (transformedLength == 0)
    {
        // Java: Transformers decided to skip the class
        free(transformed);
        return NULL;
    }

    // Java: return defineClass(name, bytes, 0, bytes.length, moduleInfo.protectionDomain);
    // defineClass is JVM-backed (external); a C port has no class definition.
    free(transformed);
    return NULL;
}

unsigned char *LIBMATTI_FML_ModuleClassLoader_MaybeTransformClassBytes(const unsigned char *bytes, size_t length,
                                                                      size_t *outLength)
{
    // Java: protected byte[] maybeTransformClassBytes(...) { return bytes; }
    unsigned char *copy = malloc(length == 0 ? 1 : length);
    if (length > 0) memcpy(copy, bytes, length);
    *outLength = length;
    return copy;
}

static unsigned char *default_maybe_transform(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                              size_t length, const char *name, const char *context,
                                              size_t *outLength)
{
    (void) self;
    (void) name;
    (void) context;
    return LIBMATTI_FML_ModuleClassLoader_MaybeTransformClassBytes(bytes, length, outLength);
}

void LIBMATTI_FML_ModuleClassLoader_SetMaybeTransformClassBytes(
    LIBMATTI_FML_ModuleClassLoader *classLoader,
    unsigned char *(*maybeTransformClassBytes)(LIBMATTI_FML_ModuleClassLoader *self, const unsigned char *bytes,
                                               size_t length, const char *name, const char *context,
                                               size_t *outLength))
{
    classLoader->maybeTransformClassBytes = maybeTransformClassBytes != NULL ? maybeTransformClassBytes
                                                                            : default_maybe_transform;
}

// Java: @Nullable private static String packageName(String className)
static char *package_name(const char *className)
{
    // Java: var lastSeparator = className.lastIndexOf('.'); if (lastSeparator <= 0) return null;
    const char *lastSeparator = strrchr(className, '.');
    if (lastSeparator == NULL || lastSeparator == className) return NULL;
    return strndup(className, (size_t) (lastSeparator - className));
}

void *LIBMATTI_FML_ModuleClassLoader_LoadClass(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *name,
                                              int resolve)
{
    // Java: synchronized (getClassLoadingLock(name)) {
    //          var c = findLoadedClass(name);                              // JVM (external) -> null
    //          if (c == null) { var packageName = packageName(name);
    //              if (packageName != null) { var localModule = packageLookup.get(packageName);
    //                  if (localModule != null) c = readerToClass(localModule, name);
    //                  else c = this.parentLoaders.getOrDefault(packageName, fallbackClassLoader).loadClass(name); }
    //              else c = fallbackClassLoader.loadClass(name); }
    //          if (c == null) throw new ClassNotFoundException(name);
    //          if (resolve) resolveClass(c);                               // JVM (external)
    //          return c; }
    char *packageName = package_name(name);
    if (packageName != NULL)
    {
        LIBMATTI_FML_ModuleInfo *localModule = package_module(classLoader, packageName);
        free(packageName);
        if (localModule != NULL) return reader_to_class(classLoader, localModule, name);
        // Java: parentLoaders.getOrDefault(packageName, fallbackClassLoader).loadClass(name)
        // ClassLoader.loadClass is JVM-backed (external)
        return NULL;
    }
    // Java: fallbackClassLoader.loadClass(name)
    (void) resolve;
    return NULL;
}

// Java: private Enumeration<URL> enumerateResources(String name) throws IOException
static LIBMATTI_JN_URI **enumerate_resources(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *name,
                                             size_t *count)
{
    // Java: var idx = name.lastIndexOf('/'); var pkgname = (idx == -1 || idx == name.length() - 1) ? ""
    //          : name.substring(0, idx).replace('/', '.');
    const char *lastSlash = strrchr(name, '/');
    size_t nameLength = strlen(name);
    size_t index = lastSlash == NULL ? (size_t) -1 : (size_t) (lastSlash - name);

    LIBMATTI_JN_URI **result = NULL;
    *count = 0;

    if (index != (size_t) -1 && index != nameLength - 1)
    {
        char *packageName = strndup(name, index);
        for (size_t i = 0; i < index; i++) if (packageName[i] == '/') packageName[i] = '.';
        LIBMATTI_FML_ModuleInfo *localModule = package_module(classLoader, packageName);
        free(packageName);

        if (localModule != NULL)
        {
            // Java: var url = readerToURL(localModule, name); return url != null ? singletonEnumeration(url) : emptyEnumeration()
            LIBMATTI_JN_URI *url = reader_to_url(localModule, name);
            if (url == NULL) return NULL;
            result = malloc(sizeof(*result));
            result[0] = url;
            *count = 1;
            return result;
        }
    }

    // Java: for (var moduleInfo : moduleInfoCache.values()) { var url = toURL(moduleInfo.getReader().find(name)); ... }
    for (size_t i = 0; i < classLoader->moduleInfoCount; i++)
    {
        LIBMATTI_JN_URI *url = reader_to_url(classLoader->moduleInfos[i], name);
        if (url == NULL) continue;
        result = realloc(result, sizeof(*result) * (*count + 1));
        result[(*count)++] = url;
    }
    return result;
}

LIBMATTI_JN_URI *LIBMATTI_FML_ModuleClassLoader_GetResource(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                            const char *name)
{
    // Java: var reslist = enumerateResources(name);
    //       if (reslist.hasMoreElements()) return reslist.nextElement();
    //       else return fallbackClassLoader.getResource(name);
    size_t count = 0;
    LIBMATTI_JN_URI **resources = enumerate_resources(classLoader, name, &count);
    if (count > 0)
    {
        LIBMATTI_JN_URI *first = resources[0];
        free(resources);
        return first;
    }
    free(resources);
    // ClassLoader.getResource is JVM-backed (external)
    return NULL;
}

LIBMATTI_JN_URI *LIBMATTI_FML_ModuleClassLoader_FindResourceForModule(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                                     const char *moduleName, const char *name)
{
    // Java: var localModule = moduleInfoCache.get(moduleName);
    //       if (localModule == null) return null; // This method only finds resources for locally defined modules
    LIBMATTI_FML_ModuleInfo *localModule = module_info_for(classLoader, moduleName);
    if (localModule == NULL) return NULL;
    return reader_to_url(localModule, name);
}

LIBMATTI_JN_URI **LIBMATTI_FML_ModuleClassLoader_GetResources(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                             const char *name, size_t *count)
{
    // Java: var localUrls = enumerateResources(name); var parentUrls = fallbackClassLoader.getResources(name);
    //       return local results first, then the parent's
    size_t localCount = 0;
    LIBMATTI_JN_URI **localUrls = enumerate_resources(classLoader, name, &localCount);

    LIBMATTI_JN_URI **result = malloc(sizeof(*result) * (localCount > 0 ? localCount : 1));
    for (size_t i = 0; i < localCount; i++) result[i] = localUrls[i];
    *count = localCount;
    free(localUrls);

    // ClassLoader.getResources is JVM-backed (external)
    return result;
}

LIBMATTI_JN_URI **LIBMATTI_FML_ModuleClassLoader_FindResources(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                              const char *name, size_t *count)
{
    // Java: return enumerateResources(name);
    return enumerate_resources(classLoader, name, count);
}

void *LIBMATTI_FML_ModuleClassLoader_FindClassInModule(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                       const char *moduleName, const char *name)
{
    // Java: var localModule = moduleInfoCache.get(moduleName);
    //       if (localModule != null) { try { var c = readerToClass(localModule, name); if (c != null) return c; }
    //                                  catch (ClassNotFoundException ignored) {} } return null;
    LIBMATTI_FML_ModuleInfo *localModule = module_info_for(classLoader, moduleName);
    if (localModule == NULL) return NULL;
    return reader_to_class(classLoader, localModule, name);
}

void *LIBMATTI_FML_ModuleClassLoader_FindClass(LIBMATTI_FML_ModuleClassLoader *classLoader, const char *name)
{
    // Java: var packageName = packageName(name); if (packageName != null) { var localModule = packageLookup.get(packageName);
    //          if (localModule != null) { var c = readerToClass(localModule, name); if (c != null) return c; } }
    //       throw new ClassNotFoundException(name);
    char *packageName = package_name(name);
    if (packageName != NULL)
    {
        LIBMATTI_FML_ModuleInfo *localModule = package_module(classLoader, packageName);
        free(packageName);
        if (localModule != NULL)
        {
            void *clazz = reader_to_class(classLoader, localModule, name);
            if (clazz != NULL) return clazz;
        }
    }
    return NULL; // Java: ClassNotFoundException
}

unsigned char *LIBMATTI_FML_ModuleClassLoader_GetMaybeTransformedClassBytes(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                                          const char *name, const char *context,
                                                                          size_t *outLength)
{
    // Java: byte[] bytes = new byte[0]; Throwable suppressed = null;
    //       var pname = packageName(name);
    //       if (pname != null) { var localModule = packageLookup.get(pname);
    //           if (localModule != null) bytes = getClassBytes(localModule, name);
    //           else { var parentLoader = parentLoaders.get(pname);
    //                  if (parentLoader != null) { var cname = name.replace('.', '/') + ".class";
    //                      try (var is = parentLoader.getResourceAsStream(cname)) { if (is != null) bytes = is.readAllBytes(); } } } }
    //       byte[] maybeTransformedBytes = maybeTransformClassBytes(bytes, name, context);
    //       if (maybeTransformedBytes.length == 0) throw new ClassNotFoundException(name);
    unsigned char *bytes = NULL;
    size_t byteLength = 0;
    char *packageName = package_name(name);
    if (packageName != NULL)
    {
        LIBMATTI_FML_ModuleInfo *localModule = package_module(classLoader, packageName);
        if (localModule != NULL)
        {
            bytes = get_class_bytes(localModule, name, &byteLength);
        }
        else
        {
            LIBMATTI_JL_ClassLoader *parentLoader = parent_loader_for(classLoader, packageName);
            if (parentLoader != NULL)
            {
                size_t nameLength = strlen(name);
                char *cname = malloc(nameLength + 7);
                for (size_t i = 0; i < nameLength; i++) cname[i] = name[i] == '.' ? '/' : name[i];
                strcpy(cname + nameLength, ".class");
                bytes = (unsigned char *) LIBMATTI_JL_ClassLoader_GetResourceAsStream(parentLoader, cname,
                                                                                     &byteLength);
                free(cname);
            }
        }
        free(packageName);
    }

    unsigned char *transformed = classLoader->maybeTransformClassBytes(classLoader, bytes, byteLength, name, context,
                                                                      outLength);
    free(bytes);
    if (*outLength == 0)
    {
        free(transformed);
        return NULL; // Java: ClassNotFoundException
    }
    return transformed;
}

void LIBMATTI_FML_ModuleClassLoader_SetFallbackClassLoader(LIBMATTI_FML_ModuleClassLoader *classLoader,
                                                          LIBMATTI_JL_ClassLoader *fallbackClassLoader)
{
    classLoader->fallbackClassLoader = fallbackClassLoader;
}

LIBMATTI_JL_Configuration *LIBMATTI_FML_ModuleClassLoader_GetConfiguration(
    const LIBMATTI_FML_ModuleClassLoader *classLoader)
{
    return classLoader->configuration;
}
