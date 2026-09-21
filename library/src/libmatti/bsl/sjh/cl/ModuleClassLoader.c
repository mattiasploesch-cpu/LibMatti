#include "ModuleClassLoader.h"

#include "libmatti/bsl/sjh/cl/ProtectionDomainHelper.h"
#include "libmatti/java/io/InputStream.h"

#include <stdlib.h>
#include <string.h>

// Java: Map.put with HashMap semantics - a later put overwrites an earlier one
static void packageLookupPut(LIBMATTI_CL_PackageLookup *lookup, const char *pkg, const char *moduleName)
{
    for (size_t i = 0; i < lookup->count; i++)
    {
        if (strcmp(lookup->packages[i], pkg) == 0)
        {
            free(lookup->moduleNames[i]);
            lookup->moduleNames[i] = strdup(moduleName);
            return;
        }
    }
    lookup->packages = realloc(lookup->packages, sizeof(char *) * (lookup->count + 1));
    lookup->moduleNames = realloc(lookup->moduleNames, sizeof(char *) * (lookup->count + 1));
    lookup->packages[lookup->count] = strdup(pkg);
    lookup->moduleNames[lookup->count] = strdup(moduleName);
    lookup->count++;
}

static void parentLoadersPut(LIBMATTI_CL_ParentLoaders *loaders, const char *pkg, LIBMATTI_CL_ClassLoader *classLoader)
{
    for (size_t i = 0; i < loaders->count; i++)
    {
        if (strcmp(loaders->packages[i], pkg) == 0)
        {
            loaders->classLoaders[i] = classLoader;
            return;
        }
    }
    loaders->packages = realloc(loaders->packages, sizeof(char *) * (loaders->count + 1));
    loaders->classLoaders = realloc(loaders->classLoaders, sizeof(LIBMATTI_CL_ClassLoader *) * (loaders->count + 1));
    loaders->packages[loaders->count] = strdup(pkg);
    loaders->classLoaders[loaders->count] = classLoader;
    loaders->count++;
}

static int resolvedRootsContains(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name)
{
    for (size_t i = 0; i < classLoader->resolvedRoots.count; i++)
    {
        if (strcmp(classLoader->resolvedRoots.names[i], name) == 0) return 1;
    }
    return 0;
}

static unsigned char *default_maybe_transform(LIBMATTI_CL_ModuleClassLoader *self, const unsigned char *bytes,
                                              size_t length, const char *name, const char *context,
                                              size_t *outLength);

LIBMATTI_CL_ModuleClassLoader *LIBMATTI_CL_ModuleClassLoader_New(const char *name, LIBMATTI_JL_Configuration *configuration)
{
    LIBMATTI_CL_ModuleClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_CL_ModuleClassLoader));
    classLoader->name = strdup(name);
    classLoader->configuration = configuration;
    // Java: maybeTransformClassBytes is the identity unless a subclass overrides it
    classLoader->maybeTransformClassBytes = default_maybe_transform;
    // Java: this.fallbackClassLoader = Objects.requireNonNullElse(parentLoader, ClassLoader.getPlatformClassLoader());
    // getPlatformClassLoader() is JVM-backed (external); NULL means "no fallback" (ClassNotFoundException)
    classLoader->fallbackClassLoader = NULL;

    // Java: this.resolvedRoots = configuration.modules().stream()
    //          .filter(m -> m.reference() instanceof JarModuleFinder.JarModuleReference)
    //          .peek(mod -> mod.reference().descriptor().packages().forEach(pk -> packageLookup.put(pk, mod)))
    //          .collect(toMap(mod -> mod.reference().descriptor().name(), mod -> (JarModuleReference)mod.reference()));
    // In this port the only ModuleReference implementation is JarModuleReference,
    // so the instanceof filter is always satisfied.
    size_t moduleCount = 0;
    LIBMATTI_JL_ResolvedModule **modules = LIBMATTI_JL_Configuration_Modules(configuration, &moduleCount);
    for (size_t i = 0; i < moduleCount; i++)
    {
        LIBMATTI_JL_ResolvedModule *mod = modules[i];
        const char *mname = LIBMATTI_JL_ResolvedModule_Name(mod);
        LIBMATTI_JL_ModuleReference *ref = LIBMATTI_JL_ResolvedModule_Reference(mod);

        classLoader->resolvedRoots.names = realloc(classLoader->resolvedRoots.names, sizeof(char *) * (classLoader->resolvedRoots.count + 1));
        classLoader->resolvedRoots.references = realloc(classLoader->resolvedRoots.references, sizeof(LIBMATTI_CL_JarModuleReference *) * (classLoader->resolvedRoots.count + 1));
        classLoader->resolvedRoots.names[classLoader->resolvedRoots.count] = strdup(mname);
        classLoader->resolvedRoots.references[classLoader->resolvedRoots.count] = (LIBMATTI_CL_JarModuleReference *)ref;
        classLoader->resolvedRoots.count++;

        size_t pkgCount = 0;
        char **pkgs = LIBMATTI_JL_ModuleDescriptor_Packages(LIBMATTI_JL_ModuleReference_Descriptor(ref), &pkgCount);
        for (size_t p = 0; p < pkgCount; p++)
        {
            packageLookupPut(&classLoader->packageLookup, pkgs[p], mname);
        }
    }

    // Java: for (var rm : configuration.modules()) for (var other : rm.reads()) { ... }
    //          cl = classLoaderMap.computeIfAbsent(other, findClassLoader);
    //          findClassLoader: resolvedRoots.containsKey(k.name()) ? this : parentLayers/other loaders
    //          automatic modules: descriptor.packages() -> parentLoaders.put(pn, cl)
    //          else: descriptor.exports().filter(e -> !e.isQualified() ||
    //                (e.isQualified() && other.configuration() == configuration && e.targets().contains(rm.name())))
    //                .map(Exports::source) -> parentLoaders.put(pn, cl)
    // Only modules in resolvedRoots have a class loader in this port; everything else
    // is JVM-external (parent layers / platform class loader) and maps to NULL.
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
            LIBMATTI_CL_ClassLoader *cl = resolvedRootsContains(classLoader, otherName)
                    ? (LIBMATTI_CL_ClassLoader *)classLoader : NULL;
            const LIBMATTI_JL_ModuleDescriptor *desc = LIBMATTI_JL_ModuleReference_Descriptor(LIBMATTI_JL_ResolvedModule_Reference(other));

            if (LIBMATTI_JL_ModuleDescriptor_IsAutomatic(desc))
            {
                // Java: processedAutomaticDescriptors.add(descriptor) - one per module here
                int processed = 0;
                for (size_t p = 0; p < processedCount; p++)
                {
                    if (strcmp(processedAutomatic[p], otherName) == 0) { processed = 1; break; }
                }
                if (!processed)
                {
                    processedAutomatic = realloc(processedAutomatic, sizeof(char *) * (processedCount + 1));
                    processedAutomatic[processedCount++] = strdup(otherName);
                    size_t pkgCount = 0;
                    char **pkgs = LIBMATTI_JL_ModuleDescriptor_Packages((LIBMATTI_JL_ModuleDescriptor *)desc, &pkgCount);
                    for (size_t p = 0; p < pkgCount; p++)
                    {
                        parentLoadersPut(&classLoader->parentLoaders, pkgs[p], cl);
                    }
                }
            }
            else
            {
                size_t exCount = 0;
                LIBMATTI_JL_ModuleDescriptor_Exports **exports = LIBMATTI_JL_ModuleDescriptor_GetExports((LIBMATTI_JL_ModuleDescriptor *)desc, &exCount);
                for (size_t e = 0; e < exCount; e++)
                {
                    LIBMATTI_JL_ModuleDescriptor_Exports *exp = exports[e];
                    int isQualified = exp->targetCount > 0;
                    int include = !isQualified;
                    if (isQualified)
                    {
                        int targetMatch = 0;
                        for (size_t t = 0; t < exp->targetCount; t++)
                        {
                            if (strcmp(exp->targets[t], LIBMATTI_JL_ResolvedModule_Name(rm)) == 0) { targetMatch = 1; break; }
                        }
                        include = LIBMATTI_JL_ResolvedModule_Configuration(other) == configuration && targetMatch;
                    }
                    if (include) parentLoadersPut(&classLoader->parentLoaders, exp->source, cl);
                }
            }
        }
    }
    for (size_t i = 0; i < processedCount; i++) free(processedAutomatic[i]);
    free(processedAutomatic);

    // Java: bindToLayer(this, layer) for all parent layers (reflection into JVM internals)
    // ModuleLayer.bindToLoader is JVM-internal, no C equivalent
    return classLoader;
}

void LIBMATTI_CL_ModuleClassLoader_Free(LIBMATTI_CL_ModuleClassLoader *classLoader)
{
    free(classLoader->name);
    for (size_t i = 0; i < classLoader->resolvedRoots.count; i++) free(classLoader->resolvedRoots.names[i]);
    free(classLoader->resolvedRoots.names);
    free(classLoader->resolvedRoots.references);
    for (size_t i = 0; i < classLoader->packageLookup.count; i++)
    {
        free(classLoader->packageLookup.packages[i]);
        free(classLoader->packageLookup.moduleNames[i]);
    }
    free(classLoader->packageLookup.packages);
    free(classLoader->packageLookup.moduleNames);
    for (size_t i = 0; i < classLoader->parentLoaders.count; i++) free(classLoader->parentLoaders.packages[i]);
    free(classLoader->parentLoaders.packages);
    free(classLoader->parentLoaders.classLoaders);
    free(classLoader);
}

// Java: Optional<URI> reader.find(name) -> toURL
static LIBMATTI_JN_URI *readerToURL(LIBMATTI_CL_JarModuleReader *reader, const char *name)
{
    // Java: ModuleClassLoader.toURL(reader.find(name)); IOException -> null
    return LIBMATTI_CL_JarModuleReader_Find(reader, name);
}

void *LIBMATTI_CL_ModuleClassLoader_LoadClass(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name, int resolve)
{
    // Java: synchronized (getClassLoadingLock(name)) {
    //          var c = findLoadedClass(name);        // JVM (external) -> null
    //          if (c == null) { index = name.lastIndexOf('.');
    //              if (index >= 0) { pname = name.substring(0, index);
    //                  if (packageLookup.containsKey(pname)) c = findClass(packageLookup.get(pname).name(), name);
    //                  else c = parentLoaders.getOrDefault(pname, fallbackClassLoader).loadClass(name); } }
    //          if (c == null) throw new ClassNotFoundException(name);
    //          if (resolve) resolveClass(c);          // JVM (external)
    //          return c; }
    const char *lastDot = strrchr(name, '.');
    if (lastDot != NULL)
    {
        size_t pnameLen = (size_t)(lastDot - name);
        for (size_t i = 0; i < classLoader->packageLookup.count; i++)
        {
            if (strlen(classLoader->packageLookup.packages[i]) == pnameLen &&
                strncmp(classLoader->packageLookup.packages[i], name, pnameLen) == 0)
            {
                return LIBMATTI_CL_ModuleClassLoader_FindClassInModule(classLoader, classLoader->packageLookup.moduleNames[i], name);
            }
        }
        // Java: parentLoaders.getOrDefault(pname, fallbackClassLoader).loadClass(name)
        // ClassLoader.loadClass is JVM-backed (external)
    }
    (void)resolve;
    return NULL;
}

void *LIBMATTI_CL_ModuleClassLoader_FindClass(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name)
{
    // Java: String mname = classNameToModuleName(name); if (mname != null) return findClass(mname, name);
    //       else return super.findClass(name); // ClassNotFoundException (external)
    char *mname = LIBMATTI_CL_ModuleClassLoader_ClassNameToModuleName(classLoader, name);
    if (mname == NULL) return NULL;
    void *result = LIBMATTI_CL_ModuleClassLoader_FindClassInModule(classLoader, mname, name);
    free(mname);
    return result;
}

// Java: loadFromModule(moduleName, (reader, ref) -> this.readerToClass(reader, ref, name))
typedef struct
{
    LIBMATTI_CL_ModuleClassLoader *classLoader;
    const char *name;
} LIBMATTI_CL_ReaderToClassRequest;

static void *readerToClassLookup(LIBMATTI_CL_JarModuleReader *reader, LIBMATTI_CL_JarModuleReference *ref, void *userData)
{
    LIBMATTI_CL_ReaderToClassRequest *request = userData;
    const char *name = request->name;

    // Java: bytes = maybeTransformClassBytes(getClassBytes(reader, ref, name), name, null);
    //       if (bytes.length == 0) return null;
    size_t byteLen = 0;
    unsigned char *bytes = LIBMATTI_CL_ModuleClassLoader_GetClassBytes(reader, name, &byteLen);
    size_t transformedLen = 0;
    unsigned char *transformed = request->classLoader->maybeTransformClassBytes(request->classLoader, bytes, byteLen,
                                                                               name, NULL, &transformedLen);
    free(bytes);
    if (transformedLen == 0)
    {
        free(transformed);
        return NULL;
    }

    // Java: ProtectionDomainHelper.tryDefinePackage(this, name, modroot.jar().getManifest(), ...);
    //       createCodeSource(toURL(ref.location()), modroot.jar().verifyAndGetSigners(cname, bytes));
    //       defineClass(name, bytes, 0, bytes.length, createProtectionDomain(cs, this));
    //       trySetPackageModule(cls.getPackage(), cls.getModule());
    // defineClass is JVM-backed (external); a C port has no class definition, so the
    // transformed bytes are the final deliverable here (see GetMaybeTransformedClassBytes).
    (void)ref;
    free(transformed);
    return NULL;
}

void *LIBMATTI_CL_ModuleClassLoader_FindClassInModule(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName, const char *name)
{
    // Java: try { return loadFromModule(moduleName, (reader, ref) -> this.readerToClass(reader, ref, name)); }
    //       catch (IOException e) { return null; }
    LIBMATTI_CL_ReaderToClassRequest request = {(LIBMATTI_CL_ModuleClassLoader *)classLoader, name};
    return LIBMATTI_CL_ModuleClassLoader_LoadFromModule(classLoader, moduleName, readerToClassLookup, &request);
}

char *LIBMATTI_CL_ModuleClassLoader_ClassNameToModuleName(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name)
{
    // Java: final var pname = name.substring(0, name.lastIndexOf('.'));
    //       return Optional.ofNullable(this.packageLookup.get(pname)).map(ResolvedModule::name).orElse(null);
    const char *lastDot = strrchr(name, '.');
    if (lastDot == NULL) return NULL;
    size_t pnameLen = (size_t)(lastDot - name);
    for (size_t i = 0; i < classLoader->packageLookup.count; i++)
    {
        if (strlen(classLoader->packageLookup.packages[i]) == pnameLen &&
            strncmp(classLoader->packageLookup.packages[i], name, pnameLen) == 0)
        {
            return strdup(classLoader->packageLookup.moduleNames[i]);
        }
    }
    return NULL;
}

LIBMATTI_JN_URI *LIBMATTI_CL_ModuleClassLoader_GetResource(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name)
{
    // Java: var reslist = findResourceList(name); if (!reslist.isEmpty()) return reslist.get(0);
    //       else return fallbackClassLoader.getResource(name); // JVM (external)
    size_t count = 0;
    LIBMATTI_JN_URI **reslist = LIBMATTI_CL_ModuleClassLoader_FindResourceList(classLoader, name, &count);
    if (count > 0)
    {
        LIBMATTI_JN_URI *first = reslist[0];
        free(reslist);
        return first;
    }
    free(reslist);
    return NULL;
}

// Java: findResource(moduleName, name) -> loadFromModule(moduleName, (reader, ref) -> this.readerToURL(reader, ref, name))
static void *resourceLookup(LIBMATTI_CL_JarModuleReader *reader, LIBMATTI_CL_JarModuleReference *ref, void *userData)
{
    (void)ref;
    return readerToURL(reader, (const char *)userData);
}

LIBMATTI_JN_URI *LIBMATTI_CL_ModuleClassLoader_FindResource(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName, const char *name)
{
    return (LIBMATTI_JN_URI *)LIBMATTI_CL_ModuleClassLoader_LoadFromModule(classLoader, moduleName, resourceLookup, (void *)name);
}

LIBMATTI_JN_URI **LIBMATTI_CL_ModuleClassLoader_FindResourceList(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *name, size_t *count)
{
    // Java: idx = name.lastIndexOf('/'); pkgname = (idx == -1 || idx == name.length()-1) ? "" : name.substring(0,idx).replace('/','.');
    //       module = packageLookup.get(pkgname); if (module != null) { res = findResource(module.name(), name);
    //       return res != null ? List.of(res) : List.of(); }
    const char *lastSlash = strrchr(name, '/');
    size_t idx = lastSlash == NULL ? (size_t)-1 : (size_t)(lastSlash - name);
    size_t nameLen = strlen(name);
    LIBMATTI_JN_URI **result = NULL;
    *count = 0;

    if (idx != (size_t)-1 && idx != nameLen - 1)
    {
        char *pkgname = strndup(name, idx);
        for (size_t i = 0; i < strlen(pkgname); i++) if (pkgname[i] == '/') pkgname[i] = '.';
        for (size_t i = 0; i < classLoader->packageLookup.count; i++)
        {
            if (strcmp(classLoader->packageLookup.packages[i], pkgname) == 0)
            {
                LIBMATTI_JN_URI *res = LIBMATTI_CL_ModuleClassLoader_FindResource(classLoader, classLoader->packageLookup.moduleNames[i], name);
                free(pkgname);
                if (res != NULL)
                {
                    result = malloc(sizeof(LIBMATTI_JN_URI *));
                    result[0] = res;
                    *count = 1;
                }
                return result;
            }
        }
        free(pkgname);
        return NULL;
    }

    // Java: else return resolvedRoots.values().stream().map(JarModuleReference::jar)
    //              .map(jar -> jar.findFile(name)).map(toURL).filter(Objects::nonNull).toList();
    for (size_t i = 0; i < classLoader->resolvedRoots.count; i++)
    {
        LIBMATTI_CL_JarModuleReader *reader = LIBMATTI_CL_JarModuleReference_Open(classLoader->resolvedRoots.references[i]);
        LIBMATTI_JN_URI *uri = readerToURL(reader, name);
        LIBMATTI_CL_JarModuleReader_Free(reader);
        if (uri != NULL)
        {
            result = realloc(result, sizeof(LIBMATTI_JN_URI *) * (*count + 1));
            result[*count] = uri;
            (*count)++;
        }
    }
    return result;
}

unsigned char *LIBMATTI_CL_ModuleClassLoader_GetClassBytes(LIBMATTI_CL_JarModuleReader *reader, const char *name, size_t *outLength)
{
    // Java: var cname = name.replace('.','/') + ".class";
    //       try (var istream = closeHandler(Optional.of(reader).flatMap(r -> r.open(cname)))) {
    //           return istream.map(InputStream::readAllBytes).findFirst().orElseGet(() -> new byte[0]); }
    size_t nameLen = strlen(name);
    char *cname = malloc(nameLen + 7);
    for (size_t i = 0; i < nameLen; i++) cname[i] = name[i] == '.' ? '/' : name[i];
    strcpy(cname + nameLen, ".class");

    void *stream = LIBMATTI_CL_JarModuleReader_Open(reader, cname);
    free(cname);
    if (stream == NULL)
    {
        *outLength = 0;
        return NULL;
    }

    unsigned char *bytes = LIBMATTI_JI_InputStream_ReadAllBytes(stream, outLength);
    LIBMATTI_JI_InputStream_Free(stream);
    return bytes;
}

unsigned char *LIBMATTI_CL_ModuleClassLoader_MaybeTransformClassBytes(const unsigned char *bytes, size_t length, size_t *outLength)
{
    // Java: protected byte[] maybeTransformClassBytes(...) { return bytes; }
    unsigned char *copy = malloc(length == 0 ? 1 : length);
    if (length > 0) memcpy(copy, bytes, length);
    *outLength = length;
    return copy;
}

// Java: the identity implementation is the default; TransformingClassLoader replaces it
static unsigned char *default_maybe_transform(LIBMATTI_CL_ModuleClassLoader *self, const unsigned char *bytes,
                                              size_t length, const char *name, const char *context,
                                              size_t *outLength)
{

    (void)self;
    (void)name;
    (void)context;
    return LIBMATTI_CL_ModuleClassLoader_MaybeTransformClassBytes(bytes, length, outLength);
}

void LIBMATTI_CL_ModuleClassLoader_SetMaybeTransformClassBytes(
    LIBMATTI_CL_ModuleClassLoader *classLoader,
    unsigned char *(*maybeTransformClassBytes)(LIBMATTI_CL_ModuleClassLoader *self, const unsigned char *bytes,
                                               size_t length, const char *name, const char *context,
                                               size_t *outLength))
{
    classLoader->maybeTransformClassBytes = maybeTransformClassBytes != NULL ? maybeTransformClassBytes :
                                                                             default_maybe_transform;
}

void *LIBMATTI_CL_ModuleClassLoader_LoadFromModule(const LIBMATTI_CL_ModuleClassLoader *classLoader, const char *moduleName,
                                                   LIBMATTI_CL_Lookup lookup, void *userData)
{
    // Java: var module = configuration.findModule(moduleName); if (module.isEmpty()) throw new NoSuchFileException;
    //       var ref = module.get().reference(); try (var reader = ref.open()) { return lookup.apply(reader, ref); }
    LIBMATTI_JL_ResolvedModule *module = LIBMATTI_JL_Configuration_FindModule(classLoader->configuration, moduleName);
    if (module == NULL) return NULL; // Java: NoSuchFileException
    LIBMATTI_JL_ModuleReference *ref = LIBMATTI_JL_ResolvedModule_Reference(module);
    LIBMATTI_CL_JarModuleReader *reader = LIBMATTI_CL_JarModuleReference_Open((LIBMATTI_CL_JarModuleReference *)ref);
    void *result = lookup(reader, (LIBMATTI_CL_JarModuleReference *)ref, userData);
    LIBMATTI_CL_JarModuleReader_Free(reader);
    return result;
}

// Java: getMaybeTransformedClassBytes: loadFromModule(classNameToModuleName(name), (r, ref) -> getClassBytes(r, ref, name))
typedef struct
{
    const char *name;
    size_t *outLength;
} LIBMATTI_CL_ClassBytesRequest;

static void *classBytesLookup(LIBMATTI_CL_JarModuleReader *reader, LIBMATTI_CL_JarModuleReference *ref, void *userData)
{
    (void)ref;
    LIBMATTI_CL_ClassBytesRequest *request = (LIBMATTI_CL_ClassBytesRequest *)userData;
    return LIBMATTI_CL_ModuleClassLoader_GetClassBytes(reader, request->name, request->outLength);
}

unsigned char *LIBMATTI_CL_ModuleClassLoader_GetMaybeTransformedClassBytes(LIBMATTI_CL_ModuleClassLoader *classLoader,
                                                                           const char *name, const char *context,
                                                                           size_t *outLength)
{
    // Java: byte[] bytes = new byte[0]; Throwable suppressed = null;
    //       pname = name.substring(0, name.lastIndexOf('.'));
    //       if (packageLookup.containsKey(pname))
    //           bytes = loadFromModule(classNameToModuleName(name), (r, ref) -> getClassBytes(r, ref, name));
    //       else if (parentLoaders.containsKey(pname))
    //           bytes = parentLoaders.get(pname).getResourceAsStream(cname).readAllBytes(); // JVM (external)
    //       maybeTransformedBytes = maybeTransformClassBytes(bytes, name, context);
    //       if (maybeTransformedBytes.length == 0) throw new ClassNotFoundException(name);
    unsigned char *bytes = NULL;
    size_t byteLen = 0;
    const char *lastDot = strrchr(name, '.');
    if (lastDot != NULL)
    {
        size_t pnameLen = (size_t)(lastDot - name);
        for (size_t i = 0; i < classLoader->packageLookup.count; i++)
        {
            if (strlen(classLoader->packageLookup.packages[i]) == pnameLen &&
                strncmp(classLoader->packageLookup.packages[i], name, pnameLen) == 0)
            {
                LIBMATTI_CL_ClassBytesRequest request = {name, &byteLen};
                bytes = (unsigned char *)LIBMATTI_CL_ModuleClassLoader_LoadFromModule(classLoader,
                        classLoader->packageLookup.moduleNames[i], classBytesLookup, &request);
                break;
            }
        }
        // Java: else if (parentLoaders.containsKey(pname)) ... getResourceAsStream (JVM, external)
    }

    // Java: maybeTransformClassBytes(bytes, name, context); empty -> ClassNotFoundException
    unsigned char *transformed = classLoader->maybeTransformClassBytes(classLoader, bytes, byteLen, name, context,
                                                                       outLength);
    free(bytes);
    if (*outLength == 0)
    {
        free(transformed);
        return NULL; // Java: ClassNotFoundException
    }
    return transformed;
}

void LIBMATTI_CL_ModuleClassLoader_SetFallbackClassLoader(LIBMATTI_CL_ModuleClassLoader *classLoader, LIBMATTI_CL_ClassLoader *fallbackClassLoader)
{
    classLoader->fallbackClassLoader = fallbackClassLoader;
}