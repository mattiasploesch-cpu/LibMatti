// Port of org.objectweb.asm.Attribute in its raw form: an unrecognized class,
// field or method attribute is carried as name + content bytes so that it can be
// re-emitted unchanged.

#ifndef MATTICRAFT_ASM_ATTRIBUTE_H
#define MATTICRAFT_ASM_ATTRIBUTE_H

#include <stddef.h>

typedef struct LIBMATTI_ASM_Attribute
{
    char *type;
    unsigned char *content;
    size_t length;
} LIBMATTI_ASM_Attribute;

// Java: public Attribute(String type) / Attribute(String type, byte[] content)
LIBMATTI_ASM_Attribute LIBMATTI_ASM_Attribute_New(const char *type, const unsigned char *content, size_t length);
void LIBMATTI_ASM_Attribute_Free(LIBMATTI_ASM_Attribute *attribute);

#endif //MATTICRAFT_ASM_ATTRIBUTE_H
