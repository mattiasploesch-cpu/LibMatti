// Port of net.minecraft.server.packs.PackResources (interface -> vtable) and
// net.minecraft.server.packs.AbstractPackResources.

#ifndef MATTICRAFT_MC_PACKS_PACKRESOURCES_H
#define MATTICRAFT_MC_PACKS_PACKRESOURCES_H

#include "libmatti/net/minecraft/server/packs/PackLocationInfo.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/metadata/MetadataSection.h"

#include <stddef.h>

typedef struct LIBMATTI_MC_PackResources LIBMATTI_MC_PackResources;

// Java: the names of the fixed metadata directories on the pack root
extern const char *const LIBMATTI_MC_PackResources_METADATA_MARKER; // "pack.mcmeta"
extern const char *const LIBMATTI_MC_PackResources_HIDDEN_METADATA_MARKER; // ".mcmeta"

// Java: interface PackResources - one function pointer per Java method;
// the owning struct is the first parameter (the port's `this`).
typedef struct LIBMATTI_MC_PackResources_VTable
{
    // Java: @Nullable InputStream getRootResource(String... paths) - malloc'd bytes, caller frees
    unsigned char *(*getRootResource)(LIBMATTI_MC_PackResources *self, const char *const *paths, size_t pathCount,
                                      size_t *outLength);
    // Java: InputStream open(PackType, ResourceLocation) - NULL when absent
    unsigned char *(*open)(LIBMATTI_MC_PackResources *self, int type, const char *namespace, const char *path,
                           size_t *outLength);
    // Java: InputStream openResource(IoSupplier<InputStream>) - the port re-opens by location
    unsigned char *(*openResource)(LIBMATTI_MC_PackResources *self, int type, const char *namespace, const char *path,
                                   size_t *outLength);
    // Java: void listResources(PackType, String namespace, String prefix, ResourceOutput)
    // The output walks file and directory entries:
    //   onFile(locationNamespace, locationPath, ioSupplier, hasResource, resourceLength)
    void (*listResources)(LIBMATTI_MC_PackResources *self, int type, const char *namespace, const char *prefix,
                          void *userData,
                          void (*onFile)(void *userData, const char *ns, const char *path, int hasResource,
                                         size_t resourceLength));
    // Java: Set<String> getNamespaces(PackType)
    // Returns a NULL-terminated array of malloc'd names.
    char **(*getNamespaces)(LIBMATTI_MC_PackResources *self, int type, size_t *outCount);
    // Java: <T> @Nullable T getMetadataSection(MetadataSectionSerializer<T>)
    void *(*getMetadataSection)(LIBMATTI_MC_PackResources *self,
                                const LIBMATTI_MC_MetadataSectionSerializer *serializer, char **outError);
    // Java: String packId()
    const char *(*packId)(LIBMATTI_MC_PackResources *self);
    // Java: PackLocationInfo location()
    LIBMATTI_MC_PackLocationInfo *(*location)(LIBMATTI_MC_PackResources *self);
    // Java: void close()
    void (*close)(LIBMATTI_MC_PackResources *self);
} LIBMATTI_MC_PackResources_VTable;

struct LIBMATTI_MC_PackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
};

// Java: protected final PackLocationInfo location (AbstractPackResources)
struct LIBMATTI_MC_AbstractPackResources
{
    const LIBMATTI_MC_PackResources_VTable *vtable;
    LIBMATTI_MC_PackLocationInfo *location;
};

// Java: AbstractPackResources(String packId, PackLocationInfo) - legacy signature kept
LIBMATTI_MC_PackResources *LIBMATTI_MC_AbstractPackResources_NewBase(
    const LIBMATTI_MC_PackResources_VTable *vtable, const char *packId, LIBMATTI_MC_PackLocationInfo *location);
// Java: protected abstract boolean hasResource(String path)
int LIBMATTI_MC_AbstractPackResources_HasResource(LIBMATTI_MC_PackResources *self, const char *path);

// Java: static IoSupplier<InputStream> getMetadataSectionSupplierFromRoot(String name)
unsigned char *LIBMATTI_MC_PackResources_GetMetadataSectionSupplierFromRoot(LIBMATTI_MC_PackResources *self,
                                                                            const char *name, size_t *outLength);

#endif
