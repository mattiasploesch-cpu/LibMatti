// Port of net.minecraft.server.packs.AbstractPackResources.

#include "libmatti/net/minecraft/server/packs/PackResources.h"

#include "libmatti/net/minecraft/server/packs/PackType.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_PackResources *LIBMATTI_MC_AbstractPackResources_NewBase(
    const LIBMATTI_MC_PackResources_VTable *vtable, const char *packId, LIBMATTI_MC_PackLocationInfo *location)
{
    (void) packId;
    struct LIBMATTI_MC_AbstractPackResources *self = calloc(1, sizeof(struct LIBMATTI_MC_AbstractPackResources));
    self->vtable = vtable;
    self->location = location;
    return (LIBMATTI_MC_PackResources *) self;
}

LIBMATTI_MC_PackLocationInfo *abstract_location(LIBMATTI_MC_PackResources *self)
{
    return ((struct LIBMATTI_MC_AbstractPackResources *) self)->location;
}

const char *abstract_pack_id(LIBMATTI_MC_PackResources *self)
{
    return abstract_location(self)->id;
}

int LIBMATTI_MC_AbstractPackResources_HasResource(LIBMATTI_MC_PackResources *self, const char *path)
{
    // Java: protected abstract boolean hasResource(String) - dispatched over the vtable's owner
    // The concrete packs implement it via open(); the port keeps the helper for the
    // getMetadataSectionSupplierFromRoot path.
    size_t length = 0;
    unsigned char *data = self->vtable->open(self, LIBMATTI_MC_PackType_CLIENT_RESOURCES, "", path, &length);
    if (data == NULL) return 0;
    free(data);
    return 1;
}

unsigned char *LIBMATTI_MC_PackResources_GetMetadataSectionSupplierFromRoot(LIBMATTI_MC_PackResources *self,
                                                                            const char *name, size_t *outLength)
{
    // Java: getRootResource("pack.mcmeta") -> the metadata markers only exist at the root
    const char *paths[1] = {name};
    return self->vtable->getRootResource(self, paths, 1, outLength);
}
