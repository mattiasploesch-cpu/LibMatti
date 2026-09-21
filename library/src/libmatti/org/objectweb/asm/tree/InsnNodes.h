// Port of the org.objectweb.asm.tree instruction node classes
// (InsnNode, IntInsnNode, VarInsnNode, TypeInsnNode, FieldInsnNode,
//  MethodInsnNode, InvokeDynamicInsnNode, JumpInsnNode, LabelNode,
//  LdcInsnNode, IincInsnNode, TableSwitchInsnNode, LookupSwitchInsnNode,
//  MultiANewArrayInsnNode, FrameNode, LineNumberNode).
// The C port groups these small node types in one header/implementation pair
// instead of one file per class.

#ifndef MATTICRAFT_ASM_INSNNODES_H
#define MATTICRAFT_ASM_INSNNODES_H

#include "libmatti/org/objectweb/asm/Label.h"
#include "libmatti/org/objectweb/asm/Object.h"
#include "libmatti/org/objectweb/asm/tree/AbstractInsnNode.h"

#include <stddef.h>

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
} LIBMATTI_ASMT_InsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int operand;
} LIBMATTI_ASMT_IntInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int var;
} LIBMATTI_ASMT_VarInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    char *desc;
} LIBMATTI_ASMT_TypeInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    char *owner;
    char *name;
    char *desc;
} LIBMATTI_ASMT_FieldInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    char *owner;
    char *name;
    char *desc;
    int itf;
} LIBMATTI_ASMT_MethodInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    char *name;
    char *desc;
    int bsmIndex;
    int *argumentIndices;
    size_t argumentCount;
} LIBMATTI_ASMT_InvokeDynamicInsnNode;

typedef struct LIBMATTI_ASMT_LabelNode
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    LIBMATTI_ASM_Label *label;
} LIBMATTI_ASMT_LabelNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    LIBMATTI_ASMT_LabelNode *label;
} LIBMATTI_ASMT_JumpInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    LIBMATTI_ASM_Object cst;
} LIBMATTI_ASMT_LdcInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int var;
    int increment;
} LIBMATTI_ASMT_IincInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int min;
    int max;
    LIBMATTI_ASMT_LabelNode *dflt;
    LIBMATTI_ASMT_LabelNode **labels;
    size_t labelCount;
} LIBMATTI_ASMT_TableSwitchInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    LIBMATTI_ASMT_LabelNode *dflt;
    int *keys;
    LIBMATTI_ASMT_LabelNode **labels;
    size_t labelCount;
} LIBMATTI_ASMT_LookupSwitchInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    char *desc;
    int dims;
} LIBMATTI_ASMT_MultiANewArrayInsnNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int frameType;
    LIBMATTI_ASM_Object *local;
    size_t localCount;
    LIBMATTI_ASM_Object *stack;
    size_t stackCount;
} LIBMATTI_ASMT_FrameNode;

typedef struct
{
    LIBMATTI_ASMT_AbstractInsnNode base;
    int line;
    LIBMATTI_ASM_Label *start;
} LIBMATTI_ASMT_LineNumberNode;

// Java: the node constructors
LIBMATTI_ASMT_InsnNode *LIBMATTI_ASMT_InsnNode_New(int opcode);
LIBMATTI_ASMT_IntInsnNode *LIBMATTI_ASMT_IntInsnNode_New(int opcode, int operand);
LIBMATTI_ASMT_VarInsnNode *LIBMATTI_ASMT_VarInsnNode_New(int opcode, int var);
LIBMATTI_ASMT_TypeInsnNode *LIBMATTI_ASMT_TypeInsnNode_New(int opcode, const char *desc);
LIBMATTI_ASMT_FieldInsnNode *LIBMATTI_ASMT_FieldInsnNode_New(int opcode, const char *owner, const char *name, const char *desc);
LIBMATTI_ASMT_MethodInsnNode *LIBMATTI_ASMT_MethodInsnNode_New(int opcode, const char *owner, const char *name, const char *desc, int itf);
LIBMATTI_ASMT_LabelNode *LIBMATTI_ASMT_LabelNode_New(LIBMATTI_ASM_Label *label);
LIBMATTI_ASMT_JumpInsnNode *LIBMATTI_ASMT_JumpInsnNode_New(int opcode, LIBMATTI_ASMT_LabelNode *label);
LIBMATTI_ASMT_LdcInsnNode *LIBMATTI_ASMT_LdcInsnNode_New(const LIBMATTI_ASM_Object *cst);
LIBMATTI_ASMT_IincInsnNode *LIBMATTI_ASMT_IincInsnNode_New(int var, int increment);
LIBMATTI_ASMT_MultiANewArrayInsnNode *LIBMATTI_ASMT_MultiANewArrayInsnNode_New(const char *desc, int dims);

LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_ASMT_AbstractInsnNode_Clone(const LIBMATTI_ASMT_AbstractInsnNode *node);
void LIBMATTI_ASMT_AbstractInsnNode_Free(LIBMATTI_ASMT_AbstractInsnNode *node);

#endif //MATTICRAFT_ASM_INSNNODES_H
