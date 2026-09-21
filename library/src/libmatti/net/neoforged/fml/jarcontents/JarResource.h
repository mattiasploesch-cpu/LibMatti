// Port of net.neoforged.fml.jarcontents.JarResource.
// Instances obtained from a JarResourceVisitor are reused between visits (Java note);
// retain() copies a reference that can be held on to.

#ifndef MATTICRAFT_FML_JARCONTENTS_JARRESOURCE_H
#define MATTICRAFT_FML_JARCONTENTS_JARRESOURCE_H

#include "libmatti/net/neoforged/fml/jarcontents/JarResourceAttributes.h"

#include <stddef.h>

// Java: public interface JarResource
typedef struct LIBMATTI_FML_JarResource LIBMATTI_FML_JarResource;

struct LIBMATTI_FML_JarResource
{
    void *self;

    // Java: InputStream open() throws IOException
    unsigned char *(*open)(void *self, size_t *outLength);
    // Java: JarResourceAttributes attributes() throws IOException
    void (*attributes)(void *self, LIBMATTI_FML_JarResourceAttributes *out);
    // Java: JarResource retain()
    LIBMATTI_FML_JarResource *(*retain)(void *self);
    // Java has no release; the port frees the handle a get() handed out.
    void (*freeResource)(void *self);
};

// Java: public InputStream open() throws IOException - NULL on failure
unsigned char *LIBMATTI_FML_JarResource_Open(const LIBMATTI_FML_JarResource *resource, size_t *outLength);
// Java: public default byte[] readAllBytes() throws IOException
unsigned char *LIBMATTI_FML_JarResource_ReadAllBytes(const LIBMATTI_FML_JarResource *resource, size_t *outLength);
// Java: public JarResourceAttributes attributes() throws IOException
void LIBMATTI_FML_JarResource_Attributes(const LIBMATTI_FML_JarResource *resource,
                                         LIBMATTI_FML_JarResourceAttributes *out);
// Java: public JarResource retain()
LIBMATTI_FML_JarResource *LIBMATTI_FML_JarResource_Retain(const LIBMATTI_FML_JarResource *resource);
// Java: the handle lives until the GC; the port frees what get() allocated.
void LIBMATTI_FML_JarResource_Free(LIBMATTI_FML_JarResource *resource);

#endif //MATTICRAFT_FML_JARCONTENTS_JARRESOURCE_H
