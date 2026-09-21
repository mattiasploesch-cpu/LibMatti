#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModJarMetadata.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: @Override public String name() { return modFile.getId(); }
static const char *name(void *self)
{
    return LIBMATTI_NEOFORGESPI_IModFile_GetId(((LIBMATTI_FML_ModJarMetadata *) self)->modFile);
}

// Java: @Override public String version() { return modFile.getModFileInfo().versionString(); }
static const char *version(void *self)
{
    return LIBMATTI_NEOFORGESPI_IModFileInfo_VersionString(
        LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(((LIBMATTI_FML_ModJarMetadata *) self)->modFile));
}

// Java: @Override public ModuleDescriptor createDescriptor(JarContents contents)
static LIBMATTI_JL_ModuleDescriptor *createDescriptor(void *self, LIBMATTI_FML_JarContents *contents)
{
    LIBMATTI_FML_ModJarMetadata *metadata = self;

    // Java: var bld = ModuleDescriptor.newAutomaticModule(name()).version(version());
    LIBMATTI_JL_ModuleDescriptor *builder = LIBMATTI_JL_ModuleDescriptor_Create(name(self), NULL, 0);
    LIBMATTI_JL_ModuleDescriptor_AddModifier(builder, LIBMATTI_JL_MODIFIER_AUTOMATIC);
    const char *versionString = version(self);
    if (versionString != NULL)
        LIBMATTI_JL_ModuleDescriptor_SetVersion(builder, versionString);

    // Java: JarModuleInfo.scanAutomaticModule(contents, bld, "assets", "data");
    const char *excluded[] = {"assets", "data"};
    LIBMATTI_FML_JarModuleInfo_ScanAutomaticModule(contents, builder, excluded, 2);

    // Java: modFile.getModFileInfo().usesServices().forEach(bld::uses);
    size_t serviceCount = 0;
    char **services = LIBMATTI_NEOFORGESPI_IModFileInfo_UsesServices(
        LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(metadata->modFile), &serviceCount);
    for (size_t i = 0; i < serviceCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddUses(builder, services[i]);

    // Java: return bld.build();
    return builder;
}

// Java: the GC
static void freeModuleInfo(void *self)
{
    free(self);
}

LIBMATTI_FML_ModJarMetadata *LIBMATTI_FML_ModJarMetadata_New(void)
{
    LIBMATTI_FML_ModJarMetadata *metadata = calloc(1, sizeof(LIBMATTI_FML_ModJarMetadata));
    metadata->base.self = metadata;
    metadata->base.name = name;
    metadata->base.version = version;
    metadata->base.createDescriptor = createDescriptor;
    metadata->base.free = freeModuleInfo;
    return metadata;
}

// Java: public void setModFile(IModFile file)
void LIBMATTI_FML_ModJarMetadata_SetModFile(LIBMATTI_FML_ModJarMetadata *metadata,
                                            LIBMATTI_NEOFORGESPI_IModFile *file)
{
    metadata->modFile = file;
}

// Java: public IModFile modFile()
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_ModJarMetadata_ModFile(const LIBMATTI_FML_ModJarMetadata *metadata)
{
    return metadata->modFile;
}

// Java: the instance used as a JarModuleInfo
LIBMATTI_FML_JarModuleInfo *LIBMATTI_FML_ModJarMetadata_AsJarModuleInfo(LIBMATTI_FML_ModJarMetadata *metadata)
{
    return metadata != NULL ? &metadata->base : NULL;
}

// Java: @Override public String toString() { return "ModJarMetadata[" + "modFile=" + modFile + ']'; }
char *LIBMATTI_FML_ModJarMetadata_ToString(const LIBMATTI_FML_ModJarMetadata *metadata)
{
    const char *id = metadata->modFile != NULL ? LIBMATTI_NEOFORGESPI_IModFile_GetId(metadata->modFile) : "null";
    size_t length = strlen("ModJarMetadata[modFile=]") + strlen(id) + 1;
    char *result = malloc(length);
    snprintf(result, length, "ModJarMetadata[modFile=%s]", id);
    return result;
}
