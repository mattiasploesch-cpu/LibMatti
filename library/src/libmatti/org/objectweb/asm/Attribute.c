// Port of org.objectweb.asm.Attribute (raw form).

#include "libmatti/org/objectweb/asm/Attribute.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ASM_Attribute LIBMATTI_ASM_Attribute_New(const char *type, const unsigned char *content, size_t length)
{
    LIBMATTI_ASM_Attribute attribute = {0};
    attribute.type = strdup(type);
    attribute.length = length;
    attribute.content = malloc(length > 0 ? length : 1);
    if (length > 0) memcpy(attribute.content, content, length);
    return attribute;
}

void LIBMATTI_ASM_Attribute_Free(LIBMATTI_ASM_Attribute *attribute)
{
    free(attribute->type);
    free(attribute->content);
    attribute->type = NULL;
    attribute->content = NULL;
    attribute->length = 0;
}
