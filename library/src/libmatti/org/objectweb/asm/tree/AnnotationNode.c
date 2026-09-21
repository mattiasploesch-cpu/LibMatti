// Port of org.objectweb.asm.tree.AnnotationNode.

#include "libmatti/org/objectweb/asm/tree/AnnotationNode.h"

#include <stdlib.h>
#include <string.h>

// Java: public AnnotationNode(String descriptor)
LIBMATTI_ASMT_AnnotationNode *LIBMATTI_ASMT_AnnotationNode_New(const char *descriptor)
{
    LIBMATTI_ASMT_AnnotationNode *node = calloc(1, sizeof(LIBMATTI_ASMT_AnnotationNode));
    node->desc = descriptor != NULL ? strdup(descriptor) : NULL;
    return node;
}

void LIBMATTI_ASMT_AnnotationValue_Free(LIBMATTI_ASMT_AnnotationValue *value)
{
    if (value == NULL) return;

    free(value->stringValue);
    free(value->classDescriptor);
    free(value->enumType);
    free(value->enumConst);

    if (value->annotation != NULL) LIBMATTI_ASMT_AnnotationNode_Free(value->annotation);

    for (size_t i = 0; i < value->arrayValueCount; i++)
        LIBMATTI_ASMT_AnnotationValue_Free(value->arrayValues[i]);
    free(value->arrayValues);

    free(value);
}

void LIBMATTI_ASMT_AnnotationNode_Free(LIBMATTI_ASMT_AnnotationNode *node)
{
    if (node == NULL) return;

    free(node->desc);
    for (size_t i = 0; i < node->valueCount; i++)
    {
        free(node->valueNames[i]);
        LIBMATTI_ASMT_AnnotationValue_Free(node->values[i]);
    }
    free(node->valueNames);
    free(node->values);
    free(node);
}

// Java: public void visit(String name, Object value)
void LIBMATTI_ASMT_AnnotationNode_AddValue(LIBMATTI_ASMT_AnnotationNode *node, const char *name,
                                           LIBMATTI_ASMT_AnnotationValue *value)
{
    if (node == NULL) return;

    node->valueNames = realloc(node->valueNames, sizeof(char *) * (node->valueCount + 1));
    node->values = realloc(node->values, sizeof(void *) * (node->valueCount + 1));
    node->valueNames[node->valueCount] = name != NULL ? strdup(name) : NULL;
    node->values[node->valueCount] = value;
    node->valueCount++;
}

// Java: public String desc
const char *LIBMATTI_ASMT_AnnotationNode_Desc(const LIBMATTI_ASMT_AnnotationNode *node)
{
    return node != NULL ? node->desc : NULL;
}

// Java: values.get(i)
const char *LIBMATTI_ASMT_AnnotationNode_ValueNameAt(const LIBMATTI_ASMT_AnnotationNode *node, size_t index)
{
    if (node == NULL || index >= node->valueCount) return NULL;
    return node->valueNames[index];
}

// Java: values.get(i + 1)
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationNode_ValueAt(const LIBMATTI_ASMT_AnnotationNode *node,
                                                                   size_t index)
{
    if (node == NULL || index >= node->valueCount) return NULL;
    return node->values[index];
}

// Java: for (int i = 0; i < values.size(); i += 2) { if (values.get(i).equals(name)) ... }
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationNode_FindValue(const LIBMATTI_ASMT_AnnotationNode *node,
                                                                     const char *name)
{
    if (node == NULL || name == NULL) return NULL;

    for (size_t i = 0; i < node->valueCount; i++)
        if (node->valueNames[i] != NULL && strcmp(node->valueNames[i], name) == 0)
            return node->values[i];

    return NULL;
}

// Java: ((Number) values.get(i + 1)).intValue()
int LIBMATTI_ASMT_AnnotationValue_AsInt(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->intValue : 0;
}

// Java: ((Boolean) values.get(i + 1))
int LIBMATTI_ASMT_AnnotationValue_AsBoolean(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->intValue : 0;
}

// Java: (String) values.get(i + 1)
char *LIBMATTI_ASMT_AnnotationValue_AsString(const LIBMATTI_ASMT_AnnotationValue *value)
{
    if (value == NULL || value->stringValue == NULL) return NULL;
    return strdup(value->stringValue);
}

// Java: ((String[]) values.get(i + 1))[0]
const char *LIBMATTI_ASMT_AnnotationValue_EnumType(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->enumType : NULL;
}

// Java: ((String[]) values.get(i + 1))[1]
const char *LIBMATTI_ASMT_AnnotationValue_EnumConst(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->enumConst : NULL;
}

// Java: (Type) values.get(i + 1) - its descriptor
const char *LIBMATTI_ASMT_AnnotationValue_ClassDescriptor(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->classDescriptor : NULL;
}

// Java: (AnnotationNode) values.get(i + 1)
LIBMATTI_ASMT_AnnotationNode *LIBMATTI_ASMT_AnnotationValue_Annotation(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->annotation : NULL;
}

// Java: ((List<Object>) values.get(i + 1)).size()
size_t LIBMATTI_ASMT_AnnotationValue_ArrayCount(const LIBMATTI_ASMT_AnnotationValue *value)
{
    return value != NULL ? value->arrayValueCount : 0;
}

// Java: ((List<Object>) values.get(i + 1)).get(index)
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationValue_ArrayAt(const LIBMATTI_ASMT_AnnotationValue *value,
                                                                    size_t index)
{
    if (value == NULL || index >= value->arrayValueCount) return NULL;
    return value->arrayValues[index];
}
