// Port of org.objectweb.asm.tree.ClassNode.

#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stdlib.h>

LIBMATTI_ASMT_ClassNode *LIBMATTI_ASMT_ClassNode_New(void)
{
    LIBMATTI_ASMT_ClassNode *node = calloc(1, sizeof(LIBMATTI_ASMT_ClassNode));
    node->pool = LIBMATTI_ASM_ConstantPool_New();
    return node;
}

void LIBMATTI_ASMT_ClassNode_Free(LIBMATTI_ASMT_ClassNode *node)
{
    if (node == NULL) return;

    free(node->name);
    free(node->signature);
    free(node->superName);
    free(node->sourceFile);

    for (size_t i = 0; i < node->interfaceCount; i++) free(node->interfaces[i]);
    free(node->interfaces);

    for (size_t i = 0; i < node->fieldCount; i++) LIBMATTI_ASMT_FieldNode_Free(node->fields[i]);
    free(node->fields);

    for (size_t i = 0; i < node->methodCount; i++) LIBMATTI_ASMT_MethodNode_Free(node->methods[i]);
    free(node->methods);

    for (size_t i = 0; i < node->attrCount; i++) LIBMATTI_ASM_Attribute_Free(&node->attrs[i]);
    free(node->attrs);

    for (size_t i = 0; i < node->visibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->visibleAnnotations[i]);
    free(node->visibleAnnotations);

    for (size_t i = 0; i < node->invisibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->invisibleAnnotations[i]);
    free(node->invisibleAnnotations);

    LIBMATTI_ASM_ConstantPool_Free(node->pool);
    free(node);
}

// Java: fields.add(index, node)
void LIBMATTI_ASMT_ClassNode_InsertField(LIBMATTI_ASMT_ClassNode *node, size_t index,
                                         LIBMATTI_ASMT_FieldNode *field)
{
    if (index > node->fieldCount) index = node->fieldCount;

    node->fields = realloc(node->fields, sizeof(void *) * (node->fieldCount + 1));
    for (size_t i = node->fieldCount; i > index; i--) node->fields[i] = node->fields[i - 1];
    node->fields[index] = field;
    node->fieldCount++;
}
