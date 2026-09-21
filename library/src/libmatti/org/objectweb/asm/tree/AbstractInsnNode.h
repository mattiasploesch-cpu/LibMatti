// Port of org.objectweb.asm.tree.AbstractInsnNode.

#ifndef MATTICRAFT_ASM_ABSTRACTINSNNODE_H
#define MATTICRAFT_ASM_ABSTRACTINSNNODE_H

// Java: the concrete AbstractInsnNode subclasses (the type tags ASM gives them)
typedef enum
{
    LIBMATTI_ASMT_INSN = 0,
    LIBMATTI_ASMT_INT_INSN,
    LIBMATTI_ASMT_VAR_INSN,
    LIBMATTI_ASMT_TYPE_INSN,
    LIBMATTI_ASMT_FIELD_INSN,
    LIBMATTI_ASMT_METHOD_INSN,
    LIBMATTI_ASMT_INVOKE_DYNAMIC_INSN,
    LIBMATTI_ASMT_JUMP_INSN,
    LIBMATTI_ASMT_LABEL,
    LIBMATTI_ASMT_LDC_INSN,
    LIBMATTI_ASMT_IINC_INSN,
    LIBMATTI_ASMT_TABLE_SWITCH_INSN,
    LIBMATTI_ASMT_LOOKUP_SWITCH_INSN,
    LIBMATTI_ASMT_MULTIANEWARRAY_INSN,
    LIBMATTI_ASMT_FRAME,
    LIBMATTI_ASMT_LINE_NUMBER
} LIBMATTI_ASMT_InsnType;

// Java: public abstract class AbstractInsnNode
typedef struct LIBMATTI_ASMT_AbstractInsnNode
{
    int type;
    int opcode;
    struct LIBMATTI_ASMT_AbstractInsnNode *prev;
    struct LIBMATTI_ASMT_AbstractInsnNode *next;
} LIBMATTI_ASMT_AbstractInsnNode;

// Java: public int getOpcode()
int LIBMATTI_ASM_AbstractInsnNode_GetOpcode(const LIBMATTI_ASMT_AbstractInsnNode *node);
// Java: public int getType()
int LIBMATTI_ASM_AbstractInsnNode_GetType(const LIBMATTI_ASMT_AbstractInsnNode *node);

#endif //MATTICRAFT_ASM_ABSTRACTINSNNODE_H
