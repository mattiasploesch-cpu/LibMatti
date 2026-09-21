// Port of org.objectweb.asm.tree.FieldNode.

#include "libmatti/org/objectweb/asm/tree/FieldNode.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ASMT_FieldNode *LIBMATTI_ASMT_FieldNode_New(int access, const char *name, const char *descriptor,
                                                     const char *signature, const LIBMATTI_ASM_Object *value)
{
    LIBMATTI_ASMT_FieldNode *node = calloc(1, sizeof(LIBMATTI_ASMT_FieldNode));
    node->access = access;
    node->name = strdup(name);
    node->desc = strdup(descriptor);
    node->signature = signature != NULL ? strdup(signature) : NULL;
    if (value != NULL) node->value = *value;
    return node;
}

void LIBMATTI_ASMT_FieldNode_Free(LIBMATTI_ASMT_FieldNode *node)
{
    if (node == NULL) return;

    free(node->name);
    free(node->desc);
    free(node->signature);
    LIBMATTI_ASM_Object_Free(&node->value);

    for (size_t i = 0; i < node->attrCount; i++) LIBMATTI_ASM_Attribute_Free(&node->attrs[i]);
    free(node->attrs);

    for (size_t i = 0; i < node->visibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->visibleAnnotations[i]);
    free(node->visibleAnnotations);

    for (size_t i = 0; i < node->invisibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->invisibleAnnotations[i]);
    free(node->invisibleAnnotations);

    free(node);
}
