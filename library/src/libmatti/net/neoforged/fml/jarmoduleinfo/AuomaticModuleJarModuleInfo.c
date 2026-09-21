#include "libmatti/net/neoforged/fml/jarmoduleinfo/AuomaticModuleJarModuleInfo.h"

#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleDescriptorFactory.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    LIBMATTI_FML_JarModuleInfo base;
    char *name;
    // Java: @Nullable String version
    char *version;
} AuomaticModuleJarModuleInfo;

// Java: public String name()
static const char *name(void *self)
{
    return ((AuomaticModuleJarModuleInfo *)self)->name;
}

// Java: public @Nullable String version()
static const char *version(void *self)
{
    return ((AuomaticModuleJarModuleInfo *)self)->version;
}

// Java: public ModuleDescriptor createDescriptor(JarContents contents)
static LIBMATTI_JL_ModuleDescriptor *createDescriptor(void *self, LIBMATTI_FML_JarContents *contents)
{
    AuomaticModuleJarModuleInfo *moduleInfo = self;

    // Java: var bld = ModuleDescriptor.newAutomaticModule(name());
    LIBMATTI_JL_ModuleDescriptor *builder = LIBMATTI_JL_ModuleDescriptor_Create(moduleInfo->name, NULL, 0);
    LIBMATTI_JL_ModuleDescriptor_AddModifier(builder, LIBMATTI_JL_MODIFIER_AUTOMATIC);

    // Java: if (version() != null) bld.version(version());
    if (moduleInfo->version != NULL)
        LIBMATTI_JL_ModuleDescriptor_SetVersion(builder, moduleInfo->version);

    // Java: ModuleDescriptorFactory.scanAutomaticModule(contents, bld)
    LIBMATTI_FML_ModuleDescriptorFactory_ScanAutomaticModule(contents, builder, NULL, 0);

    // Java: return bld.build()
    return builder;
}

// Java: the GC
static void freeModuleInfo(void *self)
{
    AuomaticModuleJarModuleInfo *moduleInfo = self;
    free(moduleInfo->name);
    free(moduleInfo->version);
    free(moduleInfo);
}

// Java: public static AuomaticModuleJarModuleInfo from(JarContents contents)
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_AuomaticModuleJarModuleInfo_From(LIBMATTI_FML_JarContents *contents)
{
    AuomaticModuleJarModuleInfo *moduleInfo = calloc(1, sizeof(AuomaticModuleJarModuleInfo));

    // Java: var nav = ModuleDescriptorFactory.computeNameAndVersion(contents.getPrimaryPath());
    LIBMATTI_FML_NameAndVersion *nav =
        LIBMATTI_FML_ModuleDescriptorFactory_ComputeNameAndVersion(LIBMATTI_FML_JarContents_GetPrimaryPath(contents));
    moduleInfo->name = strdup(nav->name);
    moduleInfo->version = nav->version != NULL ? strdup(nav->version) : NULL;
    LIBMATTI_FML_NameAndVersion_Free(nav);

    // Java: contents.getManifest().getMainAttributes().getValue("Automatic-Module-Name")
    const char *automaticModuleName =
        LIBMATTI_JU_Manifest_GetMainValue(LIBMATTI_FML_JarContents_GetManifest(contents), "Automatic-Module-Name");
    if (automaticModuleName != NULL)
    {
        free(moduleInfo->name);
        moduleInfo->name = strdup(automaticModuleName);
    }

    moduleInfo->base.self = moduleInfo;
    moduleInfo->base.name = name;
    moduleInfo->base.version = version;
    moduleInfo->base.createDescriptor = createDescriptor;
    moduleInfo->base.free = freeModuleInfo;
    return &moduleInfo->base;
}
