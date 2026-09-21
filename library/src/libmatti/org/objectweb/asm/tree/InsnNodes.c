// Port of org.objectweb.asm.tree.AbstractInsnNode and the node constructors.

#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ASMT_AbstractInsnNode *node_init(LIBMATTI_ASMT_AbstractInsnNode *node, int type, int opcode,
                                                 size_t size)
{
    (void)size;
    node->type = type;
    node->opcode = opcode;
    return node;
}

int LIBMATTI_ASM_AbstractInsnNode_GetOpcode(const LIBMATTI_ASMT_AbstractInsnNode *node)
{
    return node->opcode;
}

int LIBMATTI_ASM_AbstractInsnNode_GetType(const LIBMATTI_ASMT_AbstractInsnNode *node)
{
    return node->type;
}

LIBMATTI_ASMT_InsnNode *LIBMATTI_ASMT_InsnNode_New(int opcode)
{
    LIBMATTI_ASMT_InsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_InsnNode));
    node_init(&node->base, LIBMATTI_ASMT_INSN, opcode, sizeof(*node));
    return node;
}

LIBMATTI_ASMT_IntInsnNode *LIBMATTI_ASMT_IntInsnNode_New(int opcode, int operand)
{
    LIBMATTI_ASMT_IntInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_IntInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_INT_INSN, opcode, sizeof(*node));
    node->operand = operand;
    return node;
}

LIBMATTI_ASMT_VarInsnNode *LIBMATTI_ASMT_VarInsnNode_New(int opcode, int var)
{
    LIBMATTI_ASMT_VarInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_VarInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_VAR_INSN, opcode, sizeof(*node));
    node->var = var;
    return node;
}

LIBMATTI_ASMT_TypeInsnNode *LIBMATTI_ASMT_TypeInsnNode_New(int opcode, const char *desc)
{
    LIBMATTI_ASMT_TypeInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_TypeInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_TYPE_INSN, opcode, sizeof(*node));
    node->desc = strdup(desc);
    return node;
}

LIBMATTI_ASMT_FieldInsnNode *LIBMATTI_ASMT_FieldInsnNode_New(int opcode, const char *owner, const char *name,
                                                             const char *desc)
{
    LIBMATTI_ASMT_FieldInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_FieldInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_FIELD_INSN, opcode, sizeof(*node));
    node->owner = strdup(owner);
    node->name = strdup(name);
    node->desc = strdup(desc);
    return node;
}

LIBMATTI_ASMT_MethodInsnNode *LIBMATTI_ASMT_MethodInsnNode_New(int opcode, const char *owner, const char *name,
                                                               const char *desc, int itf)
{
    LIBMATTI_ASMT_MethodInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_MethodInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_METHOD_INSN, opcode, sizeof(*node));
    node->owner = strdup(owner);
    node->name = strdup(name);
    node->desc = strdup(desc);
    node->itf = itf;
    return node;
}

LIBMATTI_ASMT_LabelNode *LIBMATTI_ASMT_LabelNode_New(LIBMATTI_ASM_Label *label)
{
    LIBMATTI_ASMT_LabelNode *node = calloc(1, sizeof(LIBMATTI_ASMT_LabelNode));
    node_init(&node->base, LIBMATTI_ASMT_LABEL, -1, sizeof(*node));
    node->label = label;
    return node;
}

LIBMATTI_ASMT_JumpInsnNode *LIBMATTI_ASMT_JumpInsnNode_New(int opcode, LIBMATTI_ASMT_LabelNode *label)
{
    LIBMATTI_ASMT_JumpInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_JumpInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_JUMP_INSN, opcode, sizeof(*node));
    node->label = label;
    return node;
}

LIBMATTI_ASMT_LdcInsnNode *LIBMATTI_ASMT_LdcInsnNode_New(const LIBMATTI_ASM_Object *cst)
{
    LIBMATTI_ASMT_LdcInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_LdcInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_LDC_INSN, LIBMATTI_ASM_LDC, sizeof(*node));
    node->cst = *cst;
    return node;
}

LIBMATTI_ASMT_IincInsnNode *LIBMATTI_ASMT_IincInsnNode_New(int var, int increment)
{
    LIBMATTI_ASMT_IincInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_IincInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_IINC_INSN, LIBMATTI_ASM_IINC, sizeof(*node));
    node->var = var;
    node->increment = increment;
    return node;
}

LIBMATTI_ASMT_MultiANewArrayInsnNode *LIBMATTI_ASMT_MultiANewArrayInsnNode_New(const char *desc, int dims)
{
    LIBMATTI_ASMT_MultiANewArrayInsnNode *node = calloc(1, sizeof(LIBMATTI_ASMT_MultiANewArrayInsnNode));
    node_init(&node->base, LIBMATTI_ASMT_MULTIANEWARRAY_INSN, LIBMATTI_ASM_MULTIANEWARRAY, sizeof(*node));
    node->desc = strdup(desc);
    node->dims = dims;
    return node;
}

void LIBMATTI_ASMT_AbstractInsnNode_Free(LIBMATTI_ASMT_AbstractInsnNode *node)
{
    if (node == NULL) return;

    switch (node->type)
    {
    case LIBMATTI_ASMT_TYPE_INSN:
        free(((LIBMATTI_ASMT_TypeInsnNode *)node)->desc);
        break;
    case LIBMATTI_ASMT_FIELD_INSN:
    {
        LIBMATTI_ASMT_FieldInsnNode *field = (LIBMATTI_ASMT_FieldInsnNode *)node;
        free(field->owner);
        free(field->name);
        free(field->desc);
        break;
    }
    case LIBMATTI_ASMT_METHOD_INSN:
    {
        LIBMATTI_ASMT_MethodInsnNode *method = (LIBMATTI_ASMT_MethodInsnNode *)node;
        free(method->owner);
        free(method->name);
        free(method->desc);
        break;
    }
    case LIBMATTI_ASMT_INVOKE_DYNAMIC_INSN:
    {
        LIBMATTI_ASMT_InvokeDynamicInsnNode *dynamic = (LIBMATTI_ASMT_InvokeDynamicInsnNode *)node;
        free(dynamic->name);
        free(dynamic->desc);
        free(dynamic->argumentIndices);
        break;
    }
    case LIBMATTI_ASMT_LDC_INSN:
        LIBMATTI_ASM_Object_Free(&((LIBMATTI_ASMT_LdcInsnNode *)node)->cst);
        break;
    case LIBMATTI_ASMT_LABEL:
        LIBMATTI_ASM_Label_Free(((LIBMATTI_ASMT_LabelNode *)node)->label);
        break;
    case LIBMATTI_ASMT_TABLE_SWITCH_INSN:
        free(((LIBMATTI_ASMT_TableSwitchInsnNode *)node)->labels);
        break;
    case LIBMATTI_ASMT_LOOKUP_SWITCH_INSN:
    {
        LIBMATTI_ASMT_LookupSwitchInsnNode *lookup = (LIBMATTI_ASMT_LookupSwitchInsnNode *)node;
        free(lookup->keys);
        free(lookup->labels);
        break;
    }
    case LIBMATTI_ASMT_MULTIANEWARRAY_INSN:
        free(((LIBMATTI_ASMT_MultiANewArrayInsnNode *)node)->desc);
        break;
    case LIBMATTI_ASMT_FRAME:
    {
        LIBMATTI_ASMT_FrameNode *frame = (LIBMATTI_ASMT_FrameNode *)node;
        for (size_t i = 0; i < frame->localCount; i++) LIBMATTI_ASM_Object_Free(&frame->local[i]);
        for (size_t i = 0; i < frame->stackCount; i++) LIBMATTI_ASM_Object_Free(&frame->stack[i]);
        free(frame->local);
        free(frame->stack);
        break;
    }
    default:
        break;
    }

    free(node);
}

LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_ASMT_AbstractInsnNode_Clone(const LIBMATTI_ASMT_AbstractInsnNode *node)
{
    switch (node->type)
    {
    case LIBMATTI_ASMT_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_InsnNode_New(node->opcode);
    case LIBMATTI_ASMT_INT_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IntInsnNode_New(
            node->opcode, ((const LIBMATTI_ASMT_IntInsnNode *)node)->operand);
    case LIBMATTI_ASMT_VAR_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_VarInsnNode_New(
            node->opcode, ((const LIBMATTI_ASMT_VarInsnNode *)node)->var);
    case LIBMATTI_ASMT_TYPE_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_TypeInsnNode_New(
            node->opcode, ((const LIBMATTI_ASMT_TypeInsnNode *)node)->desc);
    case LIBMATTI_ASMT_FIELD_INSN:
    {
        const LIBMATTI_ASMT_FieldInsnNode *field = (const LIBMATTI_ASMT_FieldInsnNode *)node;
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_FieldInsnNode_New(node->opcode, field->owner,
                                                                                field->name, field->desc);
    }
    case LIBMATTI_ASMT_METHOD_INSN:
    {
        const LIBMATTI_ASMT_MethodInsnNode *method = (const LIBMATTI_ASMT_MethodInsnNode *)node;
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_MethodInsnNode_New(node->opcode, method->owner,
                                                                                  method->name, method->desc,
                                                                                  method->itf);
    }
    case LIBMATTI_ASMT_LABEL:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_LabelNode_New(
            ((const LIBMATTI_ASMT_LabelNode *)node)->label);
    case LIBMATTI_ASMT_JUMP_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_JumpInsnNode_New(
            node->opcode, ((const LIBMATTI_ASMT_JumpInsnNode *)node)->label);
    case LIBMATTI_ASMT_LDC_INSN:
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_LdcInsnNode_New(
            &((const LIBMATTI_ASMT_LdcInsnNode *)node)->cst);
    case LIBMATTI_ASMT_IINC_INSN:
    {
        const LIBMATTI_ASMT_IincInsnNode *iinc = (const LIBMATTI_ASMT_IincInsnNode *)node;
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IincInsnNode_New(iinc->var, iinc->increment);
    }
    case LIBMATTI_ASMT_MULTIANEWARRAY_INSN:
    {
        const LIBMATTI_ASMT_MultiANewArrayInsnNode *multi = (const LIBMATTI_ASMT_MultiANewArrayInsnNode *)node;
        return (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_MultiANewArrayInsnNode_New(multi->desc, multi->dims);
    }
    default:
        return NULL;
    }
}
