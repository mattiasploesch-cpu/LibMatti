#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"

#include "libmatti/net/neoforged/fml/jarmoduleinfo/AuomaticModuleJarModuleInfo.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleDescriptorFactory.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleJarModuleInfo.h"

// Java: static JarModuleInfo from(JarContents jar)
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_JarModuleInfo_From(LIBMATTI_FML_JarContents *jar)
{
    // Java: var moduleInfoResource = jar.get("module-info.class")
    LIBMATTI_FML_JarResource *moduleInfoResource = LIBMATTI_FML_JarContents_Get(jar, "module-info.class");
    if (moduleInfoResource != NULL)
        return LIBMATTI_FML_ModuleJarModuleInfo_New(moduleInfoResource);

    return LIBMATTI_FML_AuomaticModuleJarModuleInfo_From(jar);
}

// Java: static void scanAutomaticModule(JarContents jar, ModuleDescriptor.Builder builder, String... excludedRootDirectories)
void LIBMATTI_FML_JarModuleInfo_ScanAutomaticModule(LIBMATTI_FML_JarContents *jar,
                                                    LIBMATTI_JL_ModuleDescriptor *builder,
                                                    const char **excludedRootDirectories,
                                                    size_t excludedRootDirectoryCount)
{
    LIBMATTI_FML_ModuleDescriptorFactory_ScanAutomaticModule(jar, builder, excludedRootDirectories,
                                                            excludedRootDirectoryCount);
}

// Java: static Set<String> scanModulePackages(JarContents jar)
char **LIBMATTI_FML_JarModuleInfo_ScanModulePackages(LIBMATTI_FML_JarContents *jar, size_t *count)
{
    return LIBMATTI_FML_ModuleDescriptorFactory_ScanModulePackages(jar, count);
}

// Java: public String name()
const char *LIBMATTI_FML_JarModuleInfo_Name(const LIBMATTI_FML_JarModuleInfo *moduleInfo)
{
    return moduleInfo->name(moduleInfo->self);
}

// Java: public @Nullable String version()
const char *LIBMATTI_FML_JarModuleInfo_Version(const LIBMATTI_FML_JarModuleInfo *moduleInfo)
{
    return moduleInfo->version(moduleInfo->self);
}

// Java: public ModuleDescriptor createDescriptor(JarContents contents)
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_FML_JarModuleInfo_CreateDescriptor(const LIBMATTI_FML_JarModuleInfo *moduleInfo,
                                                                          LIBMATTI_FML_JarContents *contents)
{
    return moduleInfo->createDescriptor(moduleInfo->self, contents);
}

// Java: the GC
void LIBMATTI_FML_JarModuleInfo_Free(LIBMATTI_FML_JarModuleInfo *moduleInfo)
{
    if (moduleInfo == NULL)
        return;
    moduleInfo->free(moduleInfo->self);
}
