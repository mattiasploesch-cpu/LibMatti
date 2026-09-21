//
// Created by administrator on 09.09.26.
//

#include "SimpleJarMetadata.h"

#include "libmatti/bsl/sjh/jarhandling/JarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"

#include <stdlib.h>
#include <string.h>

// Java: protected ModuleDescriptor computeDescriptor()
static LIBMATTI_JL_ModuleDescriptor *simple_compute_descriptor(LIBMATTI_JH_LazyJarMetadata *self)
{
    LIBMATTI_JH_SimpleJarMetadata *meta = (LIBMATTI_JH_SimpleJarMetadata *)self;

    // Java: ModuleDescriptor.newAutomaticModule(name())
    LIBMATTI_JL_ModuleDescriptor *descriptor = LIBMATTI_JL_ModuleDescriptor_Create(meta->name, NULL, 0);
    LIBMATTI_JL_ModuleDescriptor_AddModifier(descriptor, LIBMATTI_JL_MODIFIER_AUTOMATIC);

    if (meta->version != NULL)
        LIBMATTI_JL_ModuleDescriptor_SetVersion(descriptor, meta->version);

    size_t packageCount;
    char **packages = LIBMATTI_JH_JarContents_GetPackages(meta->contents, &packageCount);
    for (size_t i = 0; i < packageCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddPackage(descriptor, packages[i]);

    for (size_t i = 0; i < meta->providerCount; i++)
        LIBMATTI_JL_ModuleDescriptor_AddProvides(descriptor, meta->providers[i].serviceName,
                                                 meta->providers[i].providers, meta->providers[i].providerCount);

    return descriptor;
}

LIBMATTI_JH_SimpleJarMetadata *LIBMATTI_JH_SimpleJarMetadata_Create(const char *name, const char *version,
                                                                    LIBMATTI_JH_JarContents *contents,
                                                                    LIBMATTI_JH_Provider *providers, size_t providerCount)
{
    LIBMATTI_JH_SimpleJarMetadata *metadata = calloc(1, sizeof(LIBMATTI_JH_SimpleJarMetadata));

    metadata->lazy.compute_descriptor = simple_compute_descriptor;
    metadata->name = strdup(name);
    if (version != NULL) metadata->version = strdup(version);
    metadata->contents = contents;

    // Java: providers.stream().filter(p -> !p.providers().isEmpty()).toList()
    metadata->providers = calloc(providerCount, sizeof(LIBMATTI_JH_Provider));
    for (size_t i = 0; i < providerCount; i++)
    {
        if (providers[i].providerCount > 0)
        {
            metadata->providers[metadata->providerCount] = providers[i];
            metadata->providerCount++;
        }
    }

    return metadata;
}

// ---------------------------------------------------------------------------
// JarMetadata interface implementation
// ---------------------------------------------------------------------------

const char *LIBMATTI_JH_JarMetadata_Name(const LIBMATTI_JH_JarMetadata *metadata)
{
    const LIBMATTI_JH_SimpleJarMetadata *meta = (const LIBMATTI_JH_SimpleJarMetadata *)metadata;
    return meta->name;
}

const char *LIBMATTI_JH_JarMetadata_Version(const LIBMATTI_JH_JarMetadata *metadata)
{
    const LIBMATTI_JH_SimpleJarMetadata *meta = (const LIBMATTI_JH_SimpleJarMetadata *)metadata;
    return meta->version;
}

LIBMATTI_JL_ModuleDescriptor *LIBMATTI_JH_JarMetadata_Descriptor(const LIBMATTI_JH_JarMetadata *metadata)
{
    const LIBMATTI_JH_SimpleJarMetadata *meta = (const LIBMATTI_JH_SimpleJarMetadata *)metadata;
    return LIBMATTI_JH_LazyJarMetadata_Descriptor((LIBMATTI_JH_LazyJarMetadata *)&meta->lazy);
}

LIBMATTI_JH_Provider *LIBMATTI_JH_JarMetadata_Providers(const LIBMATTI_JH_JarMetadata *metadata, size_t *count)
{
    const LIBMATTI_JH_SimpleJarMetadata *meta = (const LIBMATTI_JH_SimpleJarMetadata *)metadata;
    *count = meta->providerCount;
    return meta->providers;
}

void LIBMATTI_JH_JarMetadata_Free(LIBMATTI_JH_JarMetadata *metadata)
{
    LIBMATTI_JH_SimpleJarMetadata *meta = (LIBMATTI_JH_SimpleJarMetadata *)metadata;
    if (meta == NULL) return;

    // The providers are shallow copies referencing the contents' data
    free(meta->providers);
    free(meta->name);
    free(meta->version);
    LIBMATTI_JL_ModuleDescriptor_Free(meta->lazy.descriptor);
    free(meta);
}