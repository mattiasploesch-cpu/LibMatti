// Port of org.objectweb.asm.tree.MethodNode.
// The C port keeps the original Code attribute bytes so that an untouched
// method is written back verbatim; the parsed instruction list is used when a
// transformer modified the method.

#ifndef MATTICRAFT_ASMT_METHODNODE_H
#define MATTICRAFT_ASMT_METHODNODE_H

#include "libmatti/org/objectweb/asm/Attribute.h"
#include "libmatti/org/objectweb/asm/tree/AnnotationNode.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stddef.h>

// Java: public class TryCatchBlockNode
typedef struct
{
    LIBMATTI_ASMT_LabelNode *start;
    LIBMATTI_ASMT_LabelNode *end;
    LIBMATTI_ASMT_LabelNode *handler;
    char *type;
} LIBMATTI_ASMT_TryCatchBlockNode;

// Java: public class MethodNode
typedef struct LIBMATTI_ASMT_MethodNode
{
    // Java: public int access; public String name, desc, signature; public List<String> exceptions
    int access;
    char *name;
    char *desc;
    char *signature;
    char **exceptions;
    size_t exceptionCount;
    // Java: public InsnList instructions; public List<TryCatchBlockNode> tryCatchBlocks
    LIBMATTI_ASMT_InsnList instructions;
    LIBMATTI_ASMT_TryCatchBlockNode *tryCatchBlocks;
    size_t tryCatchBlockCount;
    // Java: public int maxStack, maxLocals
    int maxStack;
    int maxLocals;
    int hasCode;
    // Java: the Code attribute is kept raw as long as it was not rewritten
    unsigned char *rawCode;
    size_t rawCodeLength;
    // Java: public List<Attribute> attrs
    LIBMATTI_ASM_Attribute *attrs;
    size_t attrCount;
    // Java: public List<AnnotationNode> visibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **visibleAnnotations;
    size_t visibleAnnotationCount;
    // Java: public List<AnnotationNode> invisibleAnnotations
    LIBMATTI_ASMT_AnnotationNode **invisibleAnnotations;
    size_t invisibleAnnotationCount;
} LIBMATTI_ASMT_MethodNode;

// Java: public List<LocalVariableNode> localVariables / tryCatchBlocks - the port keeps the parsed
// try/catch blocks above; the annotation helpers mirror the List operations used on the node
void LIBMATTI_ASMT_MethodNode_AddVisibleAnnotation(LIBMATTI_ASMT_MethodNode *node,
                                                   LIBMATTI_ASMT_AnnotationNode *annotation);
void LIBMATTI_ASMT_MethodNode_AddInvisibleAnnotation(LIBMATTI_ASMT_MethodNode *node,
                                                     LIBMATTI_ASMT_AnnotationNode *annotation);

// Java: public MethodNode(int access, String name, String descriptor, String signature, String[] exceptions)
LIBMATTI_ASMT_MethodNode *LIBMATTI_ASMT_MethodNode_New(int access, const char *name, const char *descriptor,
                                                       const char *signature, char **exceptions,
                                                       size_t exceptionCount);
void LIBMATTI_ASMT_MethodNode_Free(LIBMATTI_ASMT_MethodNode *node);

// Java: public void accept(ClassVisitor) - visit the node's top-level data
void LIBMATTI_ASMT_MethodNode_Visit(const LIBMATTI_ASMT_MethodNode *node, void *visitor);

#endif //MATTICRAFT_ASMT_METHODNODE_H
