// Port of org.objectweb.asm.tree.FieldNode.

#ifndef MATTICRAFT_ASMT_FIELDNODE_H
#define MATTICRAFT_ASMT_FIELDNODE_H

#include "libmatti/org/objectweb/asm/Attribute.h"
#include "libmatti/org/objectweb/asm/Object.h"
#include "libmatti/org/objectweb/asm/tree/AnnotationNode.h"

#include <stddef.h>

// Java: public class FieldNode
typedef struct LIBMATTI_ASMT_FieldNode
{
    // Java: public int access; public String name, desc, signature; public Object value
    int access;
    char *name;
    char *desc;
    char *signature;
    LIBMATTI_ASM_Object value;
    // Java: public List<Attribute> attrs
    LIBMATTI_ASM_Attribute *attrs;
    size_t attrCount;
    // Java: public List<AnnotationNode> visibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **visibleAnnotations;
    size_t visibleAnnotationCount;
    // Java: public List<AnnotationNode> invisibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **invisibleAnnotations;
    size_t invisibleAnnotationCount;
} LIBMATTI_ASMT_FieldNode;

// Java: public FieldNode(int access, String name, String descriptor, String signature, Object value)
LIBMATTI_ASMT_FieldNode *LIBMATTI_ASMT_FieldNode_New(int access, const char *name, const char *descriptor,
                                                     const char *signature, const LIBMATTI_ASM_Object *value);
void LIBMATTI_ASMT_FieldNode_Free(LIBMATTI_ASMT_FieldNode *node);

#endif //MATTICRAFT_ASMT_FIELDNODE_H
