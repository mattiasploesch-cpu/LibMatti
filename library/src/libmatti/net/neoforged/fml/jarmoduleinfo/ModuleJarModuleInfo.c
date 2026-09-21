#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleJarModuleInfo.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/module/ModuleDescriptorRead.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleDescriptorFactory.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Logger (Java throws UncheckedIOException instead)
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

typedef struct
{
    LIBMATTI_FML_JarModuleInfo base;
    // Java: private final byte[] originalDescriptorBytes
    unsigned char *originalDescriptorBytes;
    size_t originalDescriptorByteCount;
    // Java: private final ModuleDescriptor originalDescriptor
    LIBMATTI_JL_ModuleDescriptor *originalDescriptor;
} ModuleJarModuleInfo;

// Java: public ModuleDescriptor createDescriptor(JarContents contents)
static LIBMATTI_JL_ModuleDescriptor *createDescriptor(void *self, LIBMATTI_FML_JarContents *contents)
{
    ModuleJarModuleInfo *moduleInfo = self;

    // Java: var fullDescriptor = ModuleDescriptor.read(ByteBuffer.wrap(originalDescriptorBytes),
    //             () -> ModuleDescriptorFactory.scanModulePackages(contents));
    LIBMATTI_JL_ModuleDescriptor *fullDescriptor = LIBMATTI_JL_ModuleDescriptor_Read(
        moduleInfo->originalDescriptorBytes, moduleInfo->originalDescriptorByteCount);

    // Java: we do inherit the name and version, as well as the package list
    LIBMATTI_JL_ModuleDescriptor *builder =
        LIBMATTI_JL_ModuleDescriptor_Create(moduleInfo->originalDescriptor->name, NULL, 0);
    LIBMATTI_JL_ModuleDescriptor_AddModifier(builder, LIBMATTI_JL_MODIFIER_AUTOMATIC);

    if (moduleInfo->originalDescriptor->version != NULL)
        LIBMATTI_JL_ModuleDescriptor_SetVersion(builder, moduleInfo->originalDescriptor->version->raw);

    char **packages = NULL;
    size_t packageCount = 0;
    packages = LIBMATTI_JL_ModuleDescriptor_Packages(moduleInfo->originalDescriptor, &packageCount);
    for (size_t i = 0; i < packageCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddPackage(builder, packages[i]);

    // Java: fullDescriptor.provides().forEach(builder::provides)
    size_t providesCount = 0;
    LIBMATTI_JL_ModuleDescriptor_Provides **provides =
        LIBMATTI_JL_ModuleDescriptor_GetProvides(moduleInfo->originalDescriptor, &providesCount);
    for (size_t i = 0; i < providesCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddProvides(builder, provides[i]->service, provides[i]->providers,
                                                 provides[i]->providerCount);

    (void) contents;
    return builder;
}

// Java: public String name() { return originalDescriptor.name(); }
static const char *name(void *self)
{
    return ((ModuleJarModuleInfo *)self)->originalDescriptor->name;
}

// Java: public @Nullable String version() { return originalDescriptor.rawVersion().orElse(null); }
static const char *version(void *self)
{
    LIBMATTI_JL_ModuleDescriptor_Version *v = ((ModuleJarModuleInfo *)self)->originalDescriptor->version;
    return v != NULL ? v->raw : NULL;
}

// Java: the GC
static void freeModuleInfo(void *self)
{
    ModuleJarModuleInfo *moduleInfo = self;
    free(moduleInfo->originalDescriptorBytes);
    LIBMATTI_JL_ModuleDescriptor_Free(moduleInfo->originalDescriptor);
    free(moduleInfo);
}

// Java: public ModuleJarModuleInfo(JarResource moduleInfo)
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_ModuleJarModuleInfo_New(LIBMATTI_FML_JarResource *moduleInfo)
{
    ModuleJarModuleInfo *result = calloc(1, sizeof(ModuleJarModuleInfo));

    // Java: this.originalDescriptorBytes = moduleInfo.readAllBytes()
    // Java throws UncheckedIOException("Failed to read module-info.class from " + moduleInfo)
    // on a read failure; the port logs the same text and continues with NULL bytes, which
    // makes the descriptor parse below fail the same way.
    result->originalDescriptorBytes =
        LIBMATTI_FML_JarResource_ReadAllBytes(moduleInfo, &result->originalDescriptorByteCount);
    if (result->originalDescriptorBytes == NULL)
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to read module-info.class from {}", "JarResource");

    // Java: this.originalDescriptor = ModuleDescriptor.read(ByteBuffer.wrap(originalDescriptorBytes))
    LIBMATTI_JL_ModuleDescriptor *originalDescriptor = LIBMATTI_JL_ModuleDescriptor_Read(
        result->originalDescriptorBytes, result->originalDescriptorByteCount);
    if (originalDescriptor != NULL)
    {
        result->originalDescriptor = originalDescriptor;
    }
    else
    {
        // Java: InvalidModuleDescriptorException wrapped in UncheckedIOException
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to parse module-info.class: InvalidModuleDescriptorException");
        result->originalDescriptor = LIBMATTI_JL_ModuleDescriptor_Create("", NULL, 0);
    }

    result->base.self = result;
    result->base.name = name;
    result->base.version = version;
    result->base.createDescriptor = createDescriptor;
    result->base.free = freeModuleInfo;
    return &result->base;
}
