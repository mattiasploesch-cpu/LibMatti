#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"

// Java: public InputStream open() throws IOException
unsigned char *LIBMATTI_FML_JarResource_Open(const LIBMATTI_FML_JarResource *resource, size_t *outLength)
{
    return resource->open(resource->self, outLength);
}

// Java: public default byte[] readAllBytes() throws IOException
unsigned char *LIBMATTI_FML_JarResource_ReadAllBytes(const LIBMATTI_FML_JarResource *resource, size_t *outLength)
{
    return resource->open(resource->self, outLength);
}

// Java: public JarResourceAttributes attributes() throws IOException
void LIBMATTI_FML_JarResource_Attributes(const LIBMATTI_FML_JarResource *resource,
                                         LIBMATTI_FML_JarResourceAttributes *out)
{
    resource->attributes(resource->self, out);
}

// Java: public JarResource retain()
LIBMATTI_FML_JarResource *LIBMATTI_FML_JarResource_Retain(const LIBMATTI_FML_JarResource *resource)
{
    return resource->retain(resource->self);
}

// Java: the resource is collectable; the port frees the handle a get() allocated.
void LIBMATTI_FML_JarResource_Free(LIBMATTI_FML_JarResource *resource)
{
    if (resource == NULL)
        return;

    resource->freeResource(resource->self);
}
