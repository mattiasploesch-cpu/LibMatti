// Port of net.neoforged.fml.loading.mixin.FMLModFileContainerHandle.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLModFileContainerHandle.h"

#include "libmatti/java/util/jar/Manifest.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdlib.h>

// Java: public String getAttribute(String name)
//       { return modFile.getContents().getManifest().getMainAttributes().getValue(name); }
static const char *get_attribute(void *self, const char *name)
{
    LIBMATTI_FML_FMLModFileContainerHandle *handle = self;
    LIBMATTI_FML_JarContents *contents = LIBMATTI_NEOFORGESPI_IModFile_GetContents(handle->modFile);
    if (contents == NULL) return NULL;

    LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(contents);
    if (manifest == NULL) return NULL;

    return LIBMATTI_JU_Manifest_GetMainValue(manifest, name);
}

// Java: public Collection<IContainerHandle> getNestedContainers() { return List.of(); }
static LIBMATTI_SP_ContainerHandle **get_nested_containers(void *self, size_t *count)
{
    (void) self;
    *count = 0;
    return NULL;
}

// Java: public String getId() { return modFile.getId(); }
static const char *get_id(void *self)
{
    LIBMATTI_FML_FMLModFileContainerHandle *handle = self;
    return LIBMATTI_NEOFORGESPI_IModFile_GetId(handle->modFile);
}

// Java: public String getDescription() { return modFile.toString(); }
static const char *get_description(void *self)
{
    LIBMATTI_FML_FMLModFileContainerHandle *handle = self;
    return LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(handle->modFile);
}

// Java: public FMLModFileContainerHandle(IModFile modFile)
LIBMATTI_FML_FMLModFileContainerHandle *LIBMATTI_FML_FMLModFileContainerHandle_New(
    LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    LIBMATTI_FML_FMLModFileContainerHandle *handle = calloc(1, sizeof(*handle));
    handle->modFile = modFile;

    handle->base.self = handle;
    handle->base.getId = get_id;
    handle->base.getDescription = get_description;
    handle->base.getAttribute = get_attribute;
    handle->base.getNestedContainers = get_nested_containers;
    return handle;
}

void LIBMATTI_FML_FMLModFileContainerHandle_Free(LIBMATTI_FML_FMLModFileContainerHandle *handle)
{
    if (handle == NULL) return;
    free(handle);
}
