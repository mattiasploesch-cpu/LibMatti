// Port of org.objectweb.asm.tree.MethodNode.

#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ASMT_MethodNode *LIBMATTI_ASMT_MethodNode_New(int access, const char *name, const char *descriptor,
                                                       const char *signature, char **exceptions, size_t exceptionCount)
{
    LIBMATTI_ASMT_MethodNode *node = calloc(1, sizeof(LIBMATTI_ASMT_MethodNode));
    node->access = access;
    node->name = strdup(name);
    node->desc = strdup(descriptor);
    node->signature = signature != NULL ? strdup(signature) : NULL;

    if (exceptionCount > 0)
    {
        node->exceptions = malloc(sizeof(*node->exceptions) * exceptionCount);
        for (size_t i = 0; i < exceptionCount; i++) node->exceptions[i] = strdup(exceptions[i]);
        node->exceptionCount = exceptionCount;
    }

    return node;
}

void LIBMATTI_ASMT_MethodNode_Free(LIBMATTI_ASMT_MethodNode *node)
{
    if (node == NULL) return;

    free(node->name);
    free(node->desc);
    free(node->signature);

    for (size_t i = 0; i < node->exceptionCount; i++) free(node->exceptions[i]);
    free(node->exceptions);

    LIBMATTI_ASM_InsnList_Free(&node->instructions);

    for (size_t i = 0; i < node->tryCatchBlockCount; i++) free(node->tryCatchBlocks[i].type);
    free(node->tryCatchBlocks);

    for (size_t i = 0; i < node->attrCount; i++) LIBMATTI_ASM_Attribute_Free(&node->attrs[i]);
    free(node->attrs);

    for (size_t i = 0; i < node->visibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->visibleAnnotations[i]);
    free(node->visibleAnnotations);

    for (size_t i = 0; i < node->invisibleAnnotationCount; i++)
        LIBMATTI_ASMT_AnnotationNode_Free(node->invisibleAnnotations[i]);
    free(node->invisibleAnnotations);

    free(node->rawCode);
    free(node);
}

// Java: public List<AnnotationNode> visibleAnnotations
void LIBMATTI_ASMT_MethodNode_AddVisibleAnnotation(LIBMATTI_ASMT_MethodNode *node,
                                                   LIBMATTI_ASMT_AnnotationNode *annotation)
{
    node->visibleAnnotations = realloc(node->visibleAnnotations,
                                       sizeof(void *) * (node->visibleAnnotationCount + 1));
    node->visibleAnnotations[node->visibleAnnotationCount++] = annotation;
}

// Java: public List<AnnotationNode> invisibleAnnotations
void LIBMATTI_ASMT_MethodNode_AddInvisibleAnnotation(LIBMATTI_ASMT_MethodNode *node,
                                                     LIBMATTI_ASMT_AnnotationNode *annotation)
{
    node->invisibleAnnotations = realloc(node->invisibleAnnotations,
                                         sizeof(void *) * (node->invisibleAnnotationCount + 1));
    node->invisibleAnnotations[node->invisibleAnnotationCount++] = annotation;
}
