// Port of org.objectweb.asm.ClassWriter.

#include "libmatti/org/objectweb/asm/ClassWriter.h"

#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned char *data;
    size_t length;
    size_t capacity;
} Buffer;

struct LIBMATTI_ASM_ClassWriter
{
    int flags;
    LIBMATTI_ASMT_ClassNode *clazz;
    LIBMATTI_ASM_ConstantPool *pool;
};

static void buffer_reserve(Buffer *buffer, size_t extra)
{
    if (buffer->length + extra <= buffer->capacity) return;
    while (buffer->capacity < buffer->length + extra) buffer->capacity = buffer->capacity == 0 ? 256 : buffer->capacity * 2;
    buffer->data = realloc(buffer->data, buffer->capacity);
}

static void write_u1(Buffer *buffer, int value)
{
    buffer_reserve(buffer, 1);
    buffer->data[buffer->length++] = (unsigned char)value;
}

static void write_u2(Buffer *buffer, int value)
{
    buffer_reserve(buffer, 2);
    buffer->data[buffer->length++] = (unsigned char)(value >> 8);
    buffer->data[buffer->length++] = (unsigned char)value;
}

static void write_u4(Buffer *buffer, long long value)
{
    buffer_reserve(buffer, 4);
    buffer->data[buffer->length++] = (unsigned char)(value >> 24);
    buffer->data[buffer->length++] = (unsigned char)(value >> 16);
    buffer->data[buffer->length++] = (unsigned char)(value >> 8);
    buffer->data[buffer->length++] = (unsigned char)value;
}

static void write_bytes(Buffer *buffer, const unsigned char *data, size_t length)
{
    buffer_reserve(buffer, length);
    memcpy(buffer->data + buffer->length, data, length);
    buffer->length += length;
}

// ---------------------------------------------------------------------------
// instruction encoding
// ---------------------------------------------------------------------------

static int is_wide_constant(const LIBMATTI_ASM_Object *object)
{
    return object->kind == LIBMATTI_ASM_OBJ_LONG || object->kind == LIBMATTI_ASM_OBJ_DOUBLE;
}

static size_t instruction_size(const LIBMATTI_ASMT_AbstractInsnNode *node, size_t offset,
                               const LIBMATTI_ASM_ConstantPool *pool)
{
    switch (node->type)
    {
    case LIBMATTI_ASMT_LABEL:
    case LIBMATTI_ASMT_LINE_NUMBER:
    case LIBMATTI_ASMT_FRAME:
        return 0;
    case LIBMATTI_ASMT_INSN:
        return 1;
    case LIBMATTI_ASMT_INT_INSN:
        return node->opcode == LIBMATTI_ASM_SIPUSH ? 3 : 2;
    case LIBMATTI_ASMT_VAR_INSN:
        return ((const LIBMATTI_ASMT_VarInsnNode *)node)->var < 256 ? 2 : 4;
    case LIBMATTI_ASMT_IINC_INSN:
    {
        const LIBMATTI_ASMT_IincInsnNode *iinc = (const LIBMATTI_ASMT_IincInsnNode *)node;
        return iinc->var < 256 && iinc->increment >= -128 && iinc->increment <= 127 ? 3 : 6;
    }
    case LIBMATTI_ASMT_TYPE_INSN:
    case LIBMATTI_ASMT_FIELD_INSN:
        return 3;
    case LIBMATTI_ASMT_METHOD_INSN:
        return node->opcode == LIBMATTI_ASM_INVOKEINTERFACE ? 5 : 3;
    case LIBMATTI_ASMT_INVOKE_DYNAMIC_INSN:
        return 5;
    case LIBMATTI_ASMT_JUMP_INSN:
        return node->opcode == LIBMATTI_ASM_GOTO_W || node->opcode == LIBMATTI_ASM_JSR_W ? 5 : 3;
    case LIBMATTI_ASMT_LDC_INSN:
    {
        const LIBMATTI_ASMT_LdcInsnNode *ldc = (const LIBMATTI_ASMT_LdcInsnNode *)node;
        if (is_wide_constant(&ldc->cst)) return 3;
        if (ldc->cst.kind == LIBMATTI_ASM_OBJ_STRING || ldc->cst.kind == LIBMATTI_ASM_OBJ_TYPE)
        {
            int index;
            if (ldc->cst.kind == LIBMATTI_ASM_OBJ_STRING)
            {
                index = LIBMATTI_ASM_ConstantPool_NewString((LIBMATTI_ASM_ConstantPool *)pool, ldc->cst.stringValue);
            }
            else
            {
                char *internalName = LIBMATTI_ASM_Type_GetInternalName(ldc->cst.typeValue);
                index = LIBMATTI_ASM_ConstantPool_NewClass((LIBMATTI_ASM_ConstantPool *)pool, internalName);
                free(internalName);
            }
            return index > 255 ? 3 : 2;
        }
        return ldc->cst.intValue >= -1 && ldc->cst.intValue <= 5 ? 2 : 3;
    }
    case LIBMATTI_ASMT_TABLE_SWITCH_INSN:
    {
        const LIBMATTI_ASMT_TableSwitchInsnNode *table = (const LIBMATTI_ASMT_TableSwitchInsnNode *)node;
        size_t padding = 3 - (offset & 3);
        return 1 + padding + 12 + table->labelCount * 4;
    }
    case LIBMATTI_ASMT_LOOKUP_SWITCH_INSN:
    {
        const LIBMATTI_ASMT_LookupSwitchInsnNode *lookup = (const LIBMATTI_ASMT_LookupSwitchInsnNode *)node;
        size_t padding = 3 - (offset & 3);
        return 1 + padding + 8 + lookup->labelCount * 8;
    }
    case LIBMATTI_ASMT_MULTIANEWARRAY_INSN:
        return 4;
    default:
        return 1;
    }
}

// Java: ClassWriter upgrades GOTO/JSR to their wide forms when the offset does not fit
static void reconcile_sizes(LIBMATTI_ASMT_MethodNode *method, const LIBMATTI_ASM_ConstantPool *pool)
{
    for (int pass = 0; pass < 4; pass++)
    {
        size_t offset = 0;
        int changed = 0;

        for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        {
            if (node->type == LIBMATTI_ASMT_LABEL) ((LIBMATTI_ASMT_LabelNode *)node)->label->offset = (int)offset;

            if (node->type == LIBMATTI_ASMT_JUMP_INSN)
            {
                const LIBMATTI_ASMT_JumpInsnNode *jump = (const LIBMATTI_ASMT_JumpInsnNode *)node;
                if (node->opcode == LIBMATTI_ASM_GOTO || node->opcode == LIBMATTI_ASM_JSR)
                {
                    int delta = jump->label->label->offset - (int)offset;
                    if (delta < -32768 || delta > 32767)
                    {
                        node->opcode = node->opcode == LIBMATTI_ASM_GOTO ? LIBMATTI_ASM_GOTO_W : LIBMATTI_ASM_JSR_W;
                        changed = 1;
                    }
                }
            }

            offset += instruction_size(node, offset, pool);
        }

        if (!changed) break;
    }
}

static void write_instruction(Buffer *code, LIBMATTI_ASMT_AbstractInsnNode *node, LIBMATTI_ASM_ConstantPool *pool)
{
    size_t offset = code->length;

    switch (node->type)
    {
    case LIBMATTI_ASMT_LABEL:
    case LIBMATTI_ASMT_LINE_NUMBER:
    case LIBMATTI_ASMT_FRAME:
        return;
    case LIBMATTI_ASMT_INSN:
        write_u1(code, node->opcode);
        return;
    case LIBMATTI_ASMT_INT_INSN:
    {
        const LIBMATTI_ASMT_IntInsnNode *intInsn = (const LIBMATTI_ASMT_IntInsnNode *)node;
        write_u1(code, node->opcode);
        if (node->opcode == LIBMATTI_ASM_SIPUSH) write_u2(code, intInsn->operand);
        else write_u1(code, intInsn->operand);
        return;
    }
    case LIBMATTI_ASMT_VAR_INSN:
    {
        const LIBMATTI_ASMT_VarInsnNode *varInsn = (const LIBMATTI_ASMT_VarInsnNode *)node;
        if (varInsn->var < 256)
        {
            write_u1(code, node->opcode);
            write_u1(code, varInsn->var);
        }
        else
        {
            write_u1(code, LIBMATTI_ASM_WIDE);
            write_u1(code, node->opcode);
            write_u2(code, varInsn->var);
        }
        return;
    }
    case LIBMATTI_ASMT_IINC_INSN:
    {
        const LIBMATTI_ASMT_IincInsnNode *iinc = (const LIBMATTI_ASMT_IincInsnNode *)node;
        if (iinc->var < 256 && iinc->increment >= -128 && iinc->increment <= 127)
        {
            write_u1(code, LIBMATTI_ASM_IINC);
            write_u1(code, iinc->var);
            write_u1(code, (signed char)iinc->increment);
        }
        else
        {
            write_u1(code, LIBMATTI_ASM_WIDE);
            write_u1(code, LIBMATTI_ASM_IINC);
            write_u2(code, iinc->var);
            write_u2(code, iinc->increment);
        }
        return;
    }
    case LIBMATTI_ASMT_TYPE_INSN:
        write_u1(code, node->opcode);
        write_u2(code, LIBMATTI_ASM_ConstantPool_NewClass(pool, ((const LIBMATTI_ASMT_TypeInsnNode *)node)->desc));
        return;
    case LIBMATTI_ASMT_FIELD_INSN:
    {
        const LIBMATTI_ASMT_FieldInsnNode *field = (const LIBMATTI_ASMT_FieldInsnNode *)node;
        write_u1(code, node->opcode);
        write_u2(code, LIBMATTI_ASM_ConstantPool_NewFieldref(pool, field->owner, field->name, field->desc));
        return;
    }
    case LIBMATTI_ASMT_METHOD_INSN:
    {
        const LIBMATTI_ASMT_MethodInsnNode *method = (const LIBMATTI_ASMT_MethodInsnNode *)node;
        write_u1(code, node->opcode);
        write_u2(code, LIBMATTI_ASM_ConstantPool_NewMethodref(pool, method->owner, method->name, method->desc,
                                                              node->opcode == LIBMATTI_ASM_INVOKEINTERFACE
                                                                  ? 1
                                                                  : method->itf));
        if (node->opcode == LIBMATTI_ASM_INVOKEINTERFACE)
        {
            // Java: the argument count (in slots) plus a zero byte
            int arguments = 1;
            const char *descriptor = method->desc;
            for (const char *c = descriptor; *c != ')' && *c != '\0'; c++)
            {
                if (*c == '[') continue;
                if (*c == 'L')
                {
                    arguments++;
                    while (*c != ';' && *c != '\0') c++;
                }
                else if (*c == 'J' || *c == 'D') arguments += 2;
                else arguments++;
            }
            write_u1(code, arguments);
            write_u1(code, 0);
        }
        return;
    }
    case LIBMATTI_ASMT_INVOKE_DYNAMIC_INSN:
        write_u1(code, node->opcode);
        write_u2(code, ((const LIBMATTI_ASMT_InvokeDynamicInsnNode *)node)->bsmIndex);
        write_u2(code, 0);
        return;
    case LIBMATTI_ASMT_JUMP_INSN:
    {
        const LIBMATTI_ASMT_JumpInsnNode *jump = (const LIBMATTI_ASMT_JumpInsnNode *)node;
        int delta = jump->label->label->offset - (int)offset;
        write_u1(code, node->opcode);
        if (node->opcode == LIBMATTI_ASM_GOTO_W || node->opcode == LIBMATTI_ASM_JSR_W) write_u4(code, delta);
        else write_u2(code, delta);
        return;
    }
    case LIBMATTI_ASMT_LDC_INSN:
    {
        const LIBMATTI_ASMT_LdcInsnNode *ldc = (const LIBMATTI_ASMT_LdcInsnNode *)node;
        int index;

        switch (ldc->cst.kind)
        {
        case LIBMATTI_ASM_OBJ_INT: index = LIBMATTI_ASM_ConstantPool_NewInteger(pool, ldc->cst.intValue); break;
        case LIBMATTI_ASM_OBJ_FLOAT: index = LIBMATTI_ASM_ConstantPool_NewFloat(pool, (float)ldc->cst.doubleValue); break;
        case LIBMATTI_ASM_OBJ_LONG: index = LIBMATTI_ASM_ConstantPool_NewLong(pool, ldc->cst.longValue); break;
        case LIBMATTI_ASM_OBJ_DOUBLE: index = LIBMATTI_ASM_ConstantPool_NewDouble(pool, ldc->cst.doubleValue); break;
        case LIBMATTI_ASM_OBJ_STRING: index = LIBMATTI_ASM_ConstantPool_NewString(pool, ldc->cst.stringValue); break;
        case LIBMATTI_ASM_OBJ_TYPE:
        {
            char *internalName = LIBMATTI_ASM_Type_GetInternalName(ldc->cst.typeValue);
            index = LIBMATTI_ASM_ConstantPool_NewClass(pool, internalName);
            free(internalName);
            break;
        }
        default: index = ldc->cst.poolIndex; break;
        }

        if (is_wide_constant(&ldc->cst))
        {
            write_u1(code, LIBMATTI_ASM_LDC2_W);
            write_u2(code, index);
        }
        else if (index > 255)
        {
            write_u1(code, LIBMATTI_ASM_LDC_W);
            write_u2(code, index);
        }
        else
        {
            write_u1(code, LIBMATTI_ASM_LDC);
            write_u1(code, index);
        }
        return;
    }
    case LIBMATTI_ASMT_TABLE_SWITCH_INSN:
    {
        const LIBMATTI_ASMT_TableSwitchInsnNode *table = (const LIBMATTI_ASMT_TableSwitchInsnNode *)node;
        write_u1(code, LIBMATTI_ASM_TABLESWITCH);
        while ((code->length & 3) != 0) write_u1(code, 0);
        write_u4(code, table->dflt->label->offset - (int)offset);
        write_u4(code, table->min);
        write_u4(code, table->max);
        for (size_t i = 0; i < table->labelCount; i++)
            write_u4(code, table->labels[i]->label->offset - (int)offset);
        return;
    }
    case LIBMATTI_ASMT_LOOKUP_SWITCH_INSN:
    {
        const LIBMATTI_ASMT_LookupSwitchInsnNode *lookup = (const LIBMATTI_ASMT_LookupSwitchInsnNode *)node;
        write_u1(code, LIBMATTI_ASM_LOOKUPSWITCH);
        while ((code->length & 3) != 0) write_u1(code, 0);
        write_u4(code, lookup->dflt->label->offset - (int)offset);
        write_u4(code, (long long)lookup->labelCount);
        for (size_t i = 0; i < lookup->labelCount; i++)
        {
            write_u4(code, lookup->keys[i]);
            write_u4(code, lookup->labels[i]->label->offset - (int)offset);
        }
        return;
    }
    case LIBMATTI_ASMT_MULTIANEWARRAY_INSN:
    {
        const LIBMATTI_ASMT_MultiANewArrayInsnNode *multi = (const LIBMATTI_ASMT_MultiANewArrayInsnNode *)node;
        write_u1(code, LIBMATTI_ASM_MULTIANEWARRAY);
        write_u2(code, LIBMATTI_ASM_ConstantPool_NewClass(pool, multi->desc));
        write_u1(code, multi->dims);
        return;
    }
    default:
        write_u1(code, node->opcode);
        return;
    }
}

static void write_frame_type_info(Buffer *attributes, LIBMATTI_ASM_ConstantPool *pool,
                                  const LIBMATTI_ASM_Object *value)
{
    switch (value->kind)
    {
    case LIBMATTI_ASM_OBJ_INT:
        write_u1(attributes, value->intValue);
        break;
    case LIBMATTI_ASM_OBJ_STRING:
        write_u1(attributes, 7);
        write_u2(attributes, LIBMATTI_ASM_ConstantPool_NewClass(pool, value->stringValue));
        break;
    case LIBMATTI_ASM_OBJ_LABEL:
        write_u1(attributes, 8);
        write_u2(attributes, value->labelValue != NULL ? value->labelValue->offset : 0);
        break;
    default:
        write_u1(attributes, 0);
        break;
    }
}

static void write_code_attribute(Buffer *attributes, LIBMATTI_ASM_ConstantPool *pool, LIBMATTI_ASMT_MethodNode *method)
{
    Buffer code = {0};
    reconcile_sizes(method, pool);

    for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        write_instruction(&code, node, pool);

    Buffer exceptionTable = {0};
    for (size_t i = 0; i < method->tryCatchBlockCount; i++)
    {
        LIBMATTI_ASMT_TryCatchBlockNode *block = &method->tryCatchBlocks[i];
        write_u2(&exceptionTable, block->start->label->offset);
        write_u2(&exceptionTable, block->end->label->offset);
        write_u2(&exceptionTable, block->handler->label->offset);
        write_u2(&exceptionTable, block->type != NULL
                      ? LIBMATTI_ASM_ConstantPool_NewClass(pool, block->type)
                      : 0);
    }

    Buffer codeAttributes = {0};
    int codeAttributeCount = 0;

    // LineNumberTable
    size_t lineCount = 0;
    for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        if (node->type == LIBMATTI_ASMT_LINE_NUMBER) lineCount++;

    if (lineCount > 0)
    {
        Buffer lines = {0};
        write_u2(&lines, (int)lineCount);
        for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        {
            if (node->type != LIBMATTI_ASMT_LINE_NUMBER) continue;
            LIBMATTI_ASMT_LineNumberNode *line = (LIBMATTI_ASMT_LineNumberNode *)node;
            write_u2(&lines, line->start->offset);
            write_u2(&lines, line->line);
        }

        write_u2(&codeAttributes, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "LineNumberTable"));
        write_u4(&codeAttributes, (long long)lines.length);
        write_bytes(&codeAttributes, lines.data, lines.length);
        free(lines.data);
        codeAttributeCount++;
    }

    // StackMapTable (expanded frames are re-emitted as full frames)
    size_t frameCount = 0;
    for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        if (node->type == LIBMATTI_ASMT_FRAME) frameCount++;

    if (frameCount > 0)
    {
        Buffer frames = {0};
        write_u2(&frames, (int)frameCount);

        int previousOffset = -1;
        for (LIBMATTI_ASMT_AbstractInsnNode *node = method->instructions.first; node != NULL; node = node->next)
        {
            if (node->type != LIBMATTI_ASMT_FRAME) continue;
            LIBMATTI_ASMT_FrameNode *frame = (LIBMATTI_ASMT_FrameNode *)node;

            int frameOffset = ((LIBMATTI_ASMT_LabelNode *)frame->base.prev)->label->offset;

            write_u1(&frames, 255);
            write_u2(&frames, frameOffset - previousOffset - 1);
            previousOffset = frameOffset;

            size_t localCount = frame->localCount;
            while (localCount > 0 && frame->local[localCount - 1].kind == LIBMATTI_ASM_OBJ_INT &&
                   frame->local[localCount - 1].intValue == 0)
                localCount--;

            write_u2(&frames, (int)localCount);
            for (size_t i = 0; i < localCount; i++) write_frame_type_info(&frames, pool, &frame->local[i]);
            write_u2(&frames, (int)frame->stackCount);
            for (size_t i = 0; i < frame->stackCount; i++) write_frame_type_info(&frames, pool, &frame->stack[i]);
        }

        write_u2(&codeAttributes, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "StackMapTable"));
        write_u4(&codeAttributes, (long long)frames.length);
        write_bytes(&codeAttributes, frames.data, frames.length);
        free(frames.data);
        codeAttributeCount++;
    }

    write_u2(attributes, method->maxStack);
    write_u2(attributes, method->maxLocals);
    write_u4(attributes, (long long)code.length);
    write_bytes(attributes, code.data, code.length);
    write_u2(attributes, (int)method->tryCatchBlockCount);
    write_bytes(attributes, exceptionTable.data, exceptionTable.length);
    write_u2(attributes, codeAttributeCount);
    write_bytes(attributes, codeAttributes.data, codeAttributes.length);

    free(code.data);
    free(exceptionTable.data);
    free(codeAttributes.data);
}

// ---------------------------------------------------------------------------
// ClassWriter
// ---------------------------------------------------------------------------

LIBMATTI_ASM_ClassWriter *LIBMATTI_ASM_ClassWriter_New(int flags, LIBMATTI_ASMT_ClassNode *clazz)
{
    LIBMATTI_ASM_ClassWriter *writer = calloc(1, sizeof(LIBMATTI_ASM_ClassWriter));
    writer->flags = flags;
    writer->clazz = clazz;
    writer->pool = clazz != NULL ? LIBMATTI_ASM_ConstantPool_Clone(clazz->pool) : LIBMATTI_ASM_ConstantPool_New();
    return writer;
}

void LIBMATTI_ASM_ClassWriter_Free(LIBMATTI_ASM_ClassWriter *writer)
{
    if (writer == NULL) return;
    LIBMATTI_ASM_ConstantPool_Free(writer->pool);
    free(writer);
}

void LIBMATTI_ASM_ClassWriter_Accept(LIBMATTI_ASM_ClassWriter *writer, LIBMATTI_ASMT_ClassNode *node)
{
    writer->clazz = node;
}

int LIBMATTI_ASM_ClassWriter_GetFlags(const LIBMATTI_ASM_ClassWriter *writer)
{
    return writer->flags;
}

LIBMATTI_ASMT_ClassNode *LIBMATTI_ASM_ClassWriter_GetClass(const LIBMATTI_ASM_ClassWriter *writer)
{
    return writer->clazz;
}

unsigned char *LIBMATTI_ASM_ClassWriter_ToByteArray(LIBMATTI_ASM_ClassWriter *writer, size_t *length)
{
    LIBMATTI_ASM_ConstantPool *pool = writer->pool;
    LIBMATTI_ASMT_ClassNode *clazz = writer->clazz;

    Buffer body = {0};

    write_u2(&body, clazz->access);
    write_u2(&body, LIBMATTI_ASM_ConstantPool_NewClass(pool, clazz->name));
    write_u2(&body, clazz->superName != NULL ? LIBMATTI_ASM_ConstantPool_NewClass(pool, clazz->superName) : 0);

    write_u2(&body, (int)clazz->interfaceCount);
    for (size_t i = 0; i < clazz->interfaceCount; i++)
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewClass(pool, clazz->interfaces[i]));

    write_u2(&body, (int)clazz->fieldCount);
    for (size_t i = 0; i < clazz->fieldCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *field = clazz->fields[i];
        write_u2(&body, field->access);
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, field->name));
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, field->desc));

        size_t attributeCount = field->attrCount + (field->signature != NULL ? 1 : 0) +
                                (field->value.kind != LIBMATTI_ASM_OBJ_NONE ? 1 : 0);
        write_u2(&body, (int)attributeCount);

        if (field->signature != NULL)
        {
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "Signature"));
            write_u4(&body, 2);
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, field->signature));
        }

        if (field->value.kind != LIBMATTI_ASM_OBJ_NONE)
        {
            int index = 0;
            switch (field->value.kind)
            {
            case LIBMATTI_ASM_OBJ_INT: index = LIBMATTI_ASM_ConstantPool_NewInteger(pool, field->value.intValue); break;
            case LIBMATTI_ASM_OBJ_FLOAT: index = LIBMATTI_ASM_ConstantPool_NewFloat(pool, (float)field->value.doubleValue); break;
            case LIBMATTI_ASM_OBJ_LONG: index = LIBMATTI_ASM_ConstantPool_NewLong(pool, field->value.longValue); break;
            case LIBMATTI_ASM_OBJ_DOUBLE: index = LIBMATTI_ASM_ConstantPool_NewDouble(pool, field->value.doubleValue); break;
            case LIBMATTI_ASM_OBJ_STRING: index = LIBMATTI_ASM_ConstantPool_NewString(pool, field->value.stringValue); break;
            default: break;
            }
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "ConstantValue"));
            write_u4(&body, 2);
            write_u2(&body, index);
        }

        for (size_t a = 0; a < field->attrCount; a++)
        {
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, field->attrs[a].type));
            write_u4(&body, (long long)field->attrs[a].length);
            write_bytes(&body, field->attrs[a].content, field->attrs[a].length);
        }
    }

    write_u2(&body, (int)clazz->methodCount);
    for (size_t i = 0; i < clazz->methodCount; i++)
    {
        LIBMATTI_ASMT_MethodNode *method = clazz->methods[i];
        write_u2(&body, method->access);
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, method->name));
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, method->desc));

        size_t attributeCount = method->attrCount + (method->signature != NULL ? 1 : 0) +
                                (method->exceptionCount > 0 ? 1 : 0) + (method->hasCode ? 1 : 0);
        write_u2(&body, (int)attributeCount);

        if (method->signature != NULL)
        {
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "Signature"));
            write_u4(&body, 2);
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, method->signature));
        }

        if (method->exceptionCount > 0)
        {
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "Exceptions"));
            write_u4(&body, (long long)(2 + method->exceptionCount * 2));
            write_u2(&body, (int)method->exceptionCount);
            for (size_t e = 0; e < method->exceptionCount; e++)
                write_u2(&body, LIBMATTI_ASM_ConstantPool_NewClass(pool, method->exceptions[e]));
        }

        if (method->hasCode)
        {
            if (!method->instructions.modified && method->rawCode != NULL)
            {
                write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "Code"));
                write_u4(&body, (long long)method->rawCodeLength);
                write_bytes(&body, method->rawCode, method->rawCodeLength);
            }
            else
            {
                Buffer code = {0};
                write_code_attribute(&code, pool, method);

                write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "Code"));
                write_u4(&body, (long long)code.length);
                write_bytes(&body, code.data, code.length);
                free(code.data);
            }
        }

        for (size_t a = 0; a < method->attrCount; a++)
        {
            write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, method->attrs[a].type));
            write_u4(&body, (long long)method->attrs[a].length);
            write_bytes(&body, method->attrs[a].content, method->attrs[a].length);
        }
    }

    size_t classAttributeCount = clazz->attrCount + (clazz->sourceFile != NULL ? 1 : 0);
    write_u2(&body, (int)classAttributeCount);

    if (clazz->sourceFile != NULL)
    {
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, "SourceFile"));
        write_u4(&body, 2);
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, clazz->sourceFile));
    }

    for (size_t a = 0; a < clazz->attrCount; a++)
    {
        write_u2(&body, LIBMATTI_ASM_ConstantPool_NewUtf8(pool, clazz->attrs[a].type));
        write_u4(&body, (long long)clazz->attrs[a].length);
        write_bytes(&body, clazz->attrs[a].content, clazz->attrs[a].length);
    }

    Buffer out = {0};
    write_u4(&out, 0xCAFEBABEL);
    // Java: ClassNode.version = (minor << 16) | major
    write_u2(&out, clazz->version >> 16);
    write_u2(&out, clazz->version & 0xFFFF);
    write_u2(&out, (int)pool->count);

    for (size_t i = 1; i < pool->count; i++)
    {
        LIBMATTI_ASM_ConstantPoolEntry *entry = &pool->entries[i];
        if (entry->tag == 0) continue;

        write_u1(&out, entry->tag);
        switch (entry->tag)
        {
        case 1:
            write_u2(&out, (int)entry->utf8Length);
            write_bytes(&out, (const unsigned char *)entry->utf8, entry->utf8Length);
            break;
        case 3:
        case 4:
            write_u4(&out, entry->bits);
            break;
        case 5:
        case 6:
            write_u4(&out, entry->bits >> 32);
            write_u4(&out, entry->bits);
            break;
        case 7:
        case 8:
        case 16:
        case 19:
        case 20:
            write_u2(&out, entry->index1);
            break;
        case 15:
            write_u1(&out, entry->index1);
            write_u2(&out, entry->index2);
            break;
        default:
            write_u2(&out, entry->index1);
            write_u2(&out, entry->index2);
            break;
        }
    }

    write_bytes(&out, body.data, body.length);
    free(body.data);

    *length = out.length;
    return out.data;
}
