// Port of org.objectweb.asm.tree.ClassNode.

#ifndef MATTICRAFT_ASMT_CLASSNODE_H
#define MATTICRAFT_ASMT_CLASSNODE_H

#include "libmatti/org/objectweb/asm/Attribute.h"
#include "libmatti/org/objectweb/asm/ConstantPool.h"
#include "libmatti/org/objectweb/asm/tree/AnnotationNode.h"
#include "libmatti/org/objectweb/asm/tree/FieldNode.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stddef.h>

// Java: public class ClassNode
typedef struct LIBMATTI_ASMT_ClassNode
{
    // Java: public int version, access; public String name, signature, superName
    int version;
    int access;
    char *name;
    char *signature;
    char *superName;
    // Java: public List<String> interfaces
    char **interfaces;
    size_t interfaceCount;
    // Java: public String sourceFile
    char *sourceFile;
    // Java: public List<FieldNode> fields; public List<MethodNode> methods
    LIBMATTI_ASMT_FieldNode **fields;
    size_t fieldCount;
    LIBMATTI_ASMT_MethodNode **methods;
    size_t methodCount;
    // Java: public List<Attribute> attrs
    LIBMATTI_ASM_Attribute *attrs;
    size_t attrCount;
    // Java: public List<AnnotationNode> visibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **visibleAnnotations;
    size_t visibleAnnotationCount;
    // Java: public List<AnnotationNode> invisibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **invisibleAnnotations;
    size_t invisibleAnnotationCount;
    // the pool the node was parsed from (see ConstantPool.h)
    LIBMATTI_ASM_ConstantPool *pool;
} LIBMATTI_ASMT_ClassNode;

// Java: fields.add(index, node) - used by class processors that prepend enum entries
void LIBMATTI_ASMT_ClassNode_InsertField(LIBMATTI_ASMT_ClassNode *node, size_t index,
                                         LIBMATTI_ASMT_FieldNode *field);

// Java: public ClassNode(int api)
LIBMATTI_ASMT_ClassNode *LIBMATTI_ASMT_ClassNode_New(void);
void LIBMATTI_ASMT_ClassNode_Free(LIBMATTI_ASMT_ClassNode *node);

#endif //MATTICRAFT_ASMT_CLASSNODE_H
