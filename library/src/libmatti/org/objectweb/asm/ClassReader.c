// Port of org.objectweb.asm.ClassReader.

#include "libmatti/org/objectweb/asm/ClassReader.h"

#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/tree/AnnotationNode.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stdlib.h>
#include <string.h>

#define CP_UTF8 1
#define CP_INTEGER 3
#define CP_FLOAT 4
#define CP_LONG 5
#define CP_DOUBLE 6
#define CP_CLASS 7
#define CP_STRING 8
#define CP_FIELDREF 9
#define CP_METHODREF 10
#define CP_INTERFACE_METHODREF 11
#define CP_NAME_AND_TYPE 12
#define CP_METHOD_HANDLE 15
#define CP_METHOD_TYPE 16
#define CP_DYNAMIC 17
#define CP_INVOKE_DYNAMIC 18
#define CP_MODULE 19
#define CP_PACKAGE 20

typedef struct
{
    const unsigned char *data;
    size_t length;
    size_t position;
    int flags;
    LIBMATTI_ASM_ConstantPool *pool;
} Cursor;

typedef struct
{
    int offset;
    LIBMATTI_ASMT_AbstractInsnNode *node;
} InsnAt;

typedef struct
{
    LIBMATTI_ASM_Label *label;
    LIBMATTI_ASMT_LabelNode *labelNode;
} LabelEntry;

typedef struct
{
    LabelEntry *entries;
    size_t count;
} LabelMap;

typedef struct
{
    LIBMATTI_ASM_Object *items;
    size_t count;
} ObjectList;

static int u1(Cursor *cursor)
{
    return cursor->data[cursor->position++];
}

static int u2(Cursor *cursor)
{
    int value = (cursor->data[cursor->position] << 8) | cursor->data[cursor->position + 1];
    cursor->position += 2;
    return value;
}

static long long u4(Cursor *cursor)
{
    // unsigned 32 bit value
    unsigned int value = ((unsigned int)cursor->data[cursor->position] << 24) |
                         ((unsigned int)cursor->data[cursor->position + 1] << 16) |
                         ((unsigned int)cursor->data[cursor->position + 2] << 8) |
                         (unsigned int)cursor->data[cursor->position + 3];
    cursor->position += 4;
    return (long long)value;
}

static const char *utf8(Cursor *cursor, int index)
{
    return LIBMATTI_ASM_ConstantPool_Utf8(cursor->pool, index);
}

// ---------------------------------------------------------------------------
// constant pool
// ---------------------------------------------------------------------------

static void read_constant_pool(Cursor *cursor)
{
    int count = u2(cursor);

    // Java: the constant pool is 1-based and may contain unused slots (Long/Double take two)
    free(cursor->pool->entries);
    cursor->pool->entries = calloc((size_t)count + 1, sizeof(LIBMATTI_ASM_ConstantPoolEntry));
    cursor->pool->count = (size_t)count;

    for (int i = 1; i < count; i++)
    {
        int tag = u1(cursor);
        LIBMATTI_ASM_ConstantPoolEntry entry = {0};
        entry.tag = tag;

        switch (tag)
        {
        case CP_UTF8:
        {
            int length = u2(cursor);
            entry.utf8Length = (size_t)length;
            entry.utf8 = malloc((size_t)length + 1);
            for (int c = 0; c < length; c++) entry.utf8[c] = (char)cursor->data[cursor->position++];
            entry.utf8[length] = '\0';
            break;
        }
        case CP_INTEGER:
            entry.bits = (unsigned int)u4(cursor);
            break;
        case CP_FLOAT:
            entry.bits = (unsigned int)u4(cursor);
            break;
        case CP_LONG:
        {
            long long high = (unsigned int)u4(cursor);
            long long low = (unsigned int)u4(cursor);
            entry.bits = (high << 32) | low;
            break;
        }
        case CP_DOUBLE:
        {
            long long high = (unsigned int)u4(cursor);
            long long low = (unsigned int)u4(cursor);
            entry.bits = (high << 32) | low;
            break;
        }
        case CP_CLASS:
        case CP_STRING:
        case CP_METHOD_TYPE:
        case CP_MODULE:
        case CP_PACKAGE:
            entry.index1 = u2(cursor);
            break;
        case CP_FIELDREF:
        case CP_METHODREF:
        case CP_INTERFACE_METHODREF:
        case CP_NAME_AND_TYPE:
        case CP_DYNAMIC:
        case CP_INVOKE_DYNAMIC:
            entry.index1 = u2(cursor);
            entry.index2 = u2(cursor);
            break;
        case CP_METHOD_HANDLE:
            entry.index1 = u1(cursor);
            entry.index2 = u2(cursor);
            break;
        default:
            break;
        }

        cursor->pool->entries[i] = entry;

        // Java: Long/Double occupy two constant pool slots
        if (tag == CP_LONG || tag == CP_DOUBLE)
        {
            i++;
            cursor->pool->entries[i] = (LIBMATTI_ASM_ConstantPoolEntry){0};
        }
    }
}

// ---------------------------------------------------------------------------
// label bookkeeping
// ---------------------------------------------------------------------------

static LIBMATTI_ASM_Label *label_for(LabelMap *map, int offset)
{
    for (size_t i = 0; i < map->count; i++) if (map->entries[i].label->offset == offset) return map->entries[i].label;

    LIBMATTI_ASM_Label *label = LIBMATTI_ASM_Label_New();
    label->offset = offset;

    map->entries = realloc(map->entries, sizeof(LabelEntry) * (map->count + 1));
    map->entries[map->count].label = label;
    map->entries[map->count].labelNode = NULL;
    map->count++;

    return label;
}

static LIBMATTI_ASMT_LabelNode *label_node_for(LabelMap *map, LIBMATTI_ASM_Label *label)
{
    for (size_t i = 0; i < map->count; i++)
    {
        if (map->entries[i].label != label) continue;
        if (map->entries[i].labelNode == NULL)
            map->entries[i].labelNode = LIBMATTI_ASMT_LabelNode_New(label);
        return map->entries[i].labelNode;
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// instructions
// ---------------------------------------------------------------------------

static size_t instruction_size(const unsigned char *code, size_t offset, size_t length)
{
    (void)length;
    int opcode = code[offset];

    if (opcode == LIBMATTI_ASM_TABLESWITCH || opcode == LIBMATTI_ASM_LOOKUPSWITCH)
    {
        size_t cursor = offset + 1;
        while ((cursor & 3) != 0) cursor++;
        if (opcode == LIBMATTI_ASM_TABLESWITCH)
        {
            int low = (code[cursor + 4] << 24) | (code[cursor + 5] << 16) | (code[cursor + 6] << 8) | code[cursor + 7];
            int high = (code[cursor + 8] << 24) | (code[cursor + 9] << 16) | (code[cursor + 10] << 8) | code[cursor + 11];
            return (cursor - offset) + 12 + (size_t)(high - low + 1) * 4;
        }
        int count = (code[cursor + 4] << 24) | (code[cursor + 5] << 16) | (code[cursor + 6] << 8) | code[cursor + 7];
        return (cursor - offset) + 8 + (size_t)count * 8;
    }

    if (opcode == LIBMATTI_ASM_WIDE) return code[offset + 1] == LIBMATTI_ASM_IINC ? 6 : 4;

    switch (opcode)
    {
    case LIBMATTI_ASM_BIPUSH:
    case LIBMATTI_ASM_LDC:
    case LIBMATTI_ASM_ILOAD: case LIBMATTI_ASM_LLOAD: case LIBMATTI_ASM_FLOAD:
    case LIBMATTI_ASM_DLOAD: case LIBMATTI_ASM_ALOAD:
    case LIBMATTI_ASM_ISTORE: case LIBMATTI_ASM_LSTORE: case LIBMATTI_ASM_FSTORE:
    case LIBMATTI_ASM_DSTORE: case LIBMATTI_ASM_ASTORE:
    case LIBMATTI_ASM_RET:
    case LIBMATTI_ASM_NEWARRAY:
        return 2;
    case LIBMATTI_ASM_SIPUSH:
    case LIBMATTI_ASM_LDC_W:
    case LIBMATTI_ASM_LDC2_W:
    case LIBMATTI_ASM_IINC:
    case LIBMATTI_ASM_IFEQ: case LIBMATTI_ASM_IFNE: case LIBMATTI_ASM_IFLT:
    case LIBMATTI_ASM_IFGE: case LIBMATTI_ASM_IFGT: case LIBMATTI_ASM_IFLE:
    case LIBMATTI_ASM_IF_ICMPEQ: case LIBMATTI_ASM_IF_ICMPNE: case LIBMATTI_ASM_IF_ICMPLT:
    case LIBMATTI_ASM_IF_ICMPGE: case LIBMATTI_ASM_IF_ICMPGT: case LIBMATTI_ASM_IF_ICMPLE:
    case LIBMATTI_ASM_IF_ACMPEQ: case LIBMATTI_ASM_IF_ACMPNE:
    case LIBMATTI_ASM_GOTO: case LIBMATTI_ASM_JSR:
    case LIBMATTI_ASM_GETSTATIC: case LIBMATTI_ASM_PUTSTATIC:
    case LIBMATTI_ASM_GETFIELD: case LIBMATTI_ASM_PUTFIELD:
    case LIBMATTI_ASM_INVOKEVIRTUAL: case LIBMATTI_ASM_INVOKESPECIAL:
    case LIBMATTI_ASM_INVOKESTATIC:
    case LIBMATTI_ASM_NEW: case LIBMATTI_ASM_ANEWARRAY:
    case LIBMATTI_ASM_CHECKCAST: case LIBMATTI_ASM_INSTANCEOF:
    case LIBMATTI_ASM_IFNULL: case LIBMATTI_ASM_IFNONNULL:
        return 3;
    case LIBMATTI_ASM_MULTIANEWARRAY:
    case LIBMATTI_ASM_INVOKEINTERFACE:
    case LIBMATTI_ASM_INVOKEDYNAMIC:
    case LIBMATTI_ASM_GOTO_W:
    case LIBMATTI_ASM_JSR_W:
        return 5;
    default:
        return 1;
    }
}

static int s2(const unsigned char *code, size_t offset)
{
    return (short)((code[offset] << 8) | code[offset + 1]);
}

static int s4(const unsigned char *code, size_t offset)
{
    return (int)((unsigned int)code[offset] << 24 | (unsigned int)code[offset + 1] << 16 |
                 (unsigned int)code[offset + 2] << 8 | (unsigned int)code[offset + 3]);
}

static void add_insn(InsnAt **list, size_t *count, int offset, LIBMATTI_ASMT_AbstractInsnNode *node)
{
    *list = realloc(*list, sizeof(InsnAt) * (*count + 1));
    (*list)[*count].offset = offset;
    (*list)[*count].node = node;
    (*count)++;
}

// Java: the running locals array is copied into every frame; the C port deep
// copies so that every frame node owns its values.
static LIBMATTI_ASM_Object object_clone(const LIBMATTI_ASM_Object *source)
{
    if (source->kind == LIBMATTI_ASM_OBJ_STRING) return LIBMATTI_ASM_Object_OfString(source->stringValue);
    if (source->kind == LIBMATTI_ASM_OBJ_TYPE) return LIBMATTI_ASM_Object_OfType(source->typeValue);
    return *source;
}

static LIBMATTI_ASM_Object ldap_constant(Cursor *cursor, int index, size_t *outSize)
{
    LIBMATTI_ASM_Object object = {0};
    *outSize = 1;

    if (index <= 0 || (size_t)index >= cursor->pool->count) return object;

    LIBMATTI_ASM_ConstantPoolEntry *entry = &cursor->pool->entries[index];

    switch (entry->tag)
    {
    case CP_INTEGER:
        object = LIBMATTI_ASM_Object_OfInt((int)entry->bits);
        break;
    case CP_FLOAT:
    {
        int bits = (int)entry->bits;
        float value = 0;
        memcpy(&value, &bits, sizeof(value));
        object = LIBMATTI_ASM_Object_OfFloat(value);
        break;
    }
    case CP_LONG:
        object = LIBMATTI_ASM_Object_OfLong(entry->bits);
        *outSize = 2;
        break;
    case CP_DOUBLE:
    {
        double value = 0;
        memcpy(&value, &entry->bits, sizeof(value));
        object = LIBMATTI_ASM_Object_OfDouble(value);
        *outSize = 2;
        break;
    }
    case CP_STRING:
        object = LIBMATTI_ASM_Object_OfString(utf8(cursor, entry->index1));
        break;
    case CP_CLASS:
        object.kind = LIBMATTI_ASM_OBJ_TYPE;
        object.typeValue = LIBMATTI_ASM_Type_GetObjectType(utf8(cursor, entry->index1));
        break;
    case CP_METHOD_TYPE:
        object.kind = LIBMATTI_ASM_OBJ_TYPE;
        object.typeValue = LIBMATTI_ASM_Type_GetMethodType(utf8(cursor, entry->index1));
        break;
    case CP_METHOD_HANDLE:
    case CP_DYNAMIC:
    case CP_INVOKE_DYNAMIC:
        object.kind = entry->tag == CP_METHOD_HANDLE ? LIBMATTI_ASM_OBJ_HANDLE : LIBMATTI_ASM_OBJ_CONSTANT_DYNAMIC;
        object.poolIndex = index;
        break;
    default:
        break;
    }

    return object;
}

static const char *member_owner(Cursor *cursor, int index)
{
    if (index <= 0 || (size_t)index >= cursor->pool->count) return NULL;
    LIBMATTI_ASM_ConstantPoolEntry *entry = &cursor->pool->entries[index];
    return LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, entry->index1);
}

static const char *member_name(Cursor *cursor, int index)
{
    if (index <= 0 || (size_t)index >= cursor->pool->count) return NULL;
    LIBMATTI_ASM_ConstantPoolEntry *entry = &cursor->pool->entries[index];
    if ((size_t)entry->index2 >= cursor->pool->count) return NULL;
    return utf8(cursor, cursor->pool->entries[entry->index2].index1);
}

static const char *member_descriptor(Cursor *cursor, int index)
{
    if (index <= 0 || (size_t)index >= cursor->pool->count) return NULL;
    LIBMATTI_ASM_ConstantPoolEntry *entry = &cursor->pool->entries[index];
    if ((size_t)entry->index2 >= cursor->pool->count) return NULL;
    return utf8(cursor, cursor->pool->entries[entry->index2].index2);
}

// Java: ClassReader.readElementValues / readElementValue
static LIBMATTI_ASMT_AnnotationValue *read_element_value(Cursor *cursor);

// Java: ClassReader.readAnnotationValue / readElementValues
static LIBMATTI_ASMT_AnnotationNode *read_annotation(Cursor *cursor)
{
    // Java: readAnnotationValues(annotationOffset + 2, ...)
    LIBMATTI_ASMT_AnnotationNode *node = LIBMATTI_ASMT_AnnotationNode_New(utf8(cursor, u2(cursor)));

    int pairCount = u2(cursor);
    for (int i = 0; i < pairCount; i++)
    {
        const char *name = utf8(cursor, u2(cursor));
        LIBMATTI_ASMT_AnnotationNode_AddValue(node, name, read_element_value(cursor));
    }

    return node;
}

// Java: ClassReader.readElementValue - one element_value of the class file format
static LIBMATTI_ASMT_AnnotationValue *read_element_value(Cursor *cursor)
{
    LIBMATTI_ASMT_AnnotationValue *value = calloc(1, sizeof(LIBMATTI_ASMT_AnnotationValue));
    int tag = u1(cursor);
    value->tag = (char) tag;

    size_t constantSize = 1;
    LIBMATTI_ASM_Object constant;

    switch (tag)
    {
        case 'B':
        case 'C':
        case 'I':
        case 'S':
        case 'Z':
            constant = ldap_constant(cursor, u2(cursor), &constantSize);
            value->intValue = constant.intValue;
            break;
        case 'J':
            constant = ldap_constant(cursor, u2(cursor), &constantSize);
            value->longValue = constant.longValue;
            break;
        case 'D':
        case 'F':
            constant = ldap_constant(cursor, u2(cursor), &constantSize);
            value->doubleValue = constant.doubleValue;
            break;
        case 's':
        {
            const char *text = utf8(cursor, u2(cursor));
            value->stringValue = text != NULL ? strdup(text) : NULL;
            break;
        }
        case 'e':
        {
            const char *type = utf8(cursor, u2(cursor));
            const char *name = utf8(cursor, u2(cursor));
            value->enumType = type != NULL ? strdup(type) : NULL;
            value->enumConst = name != NULL ? strdup(name) : NULL;
            break;
        }
        case 'c':
        {
            const char *descriptor = utf8(cursor, u2(cursor));
            value->classDescriptor = descriptor != NULL ? strdup(descriptor) : NULL;
            break;
        }
        case '@':
            value->annotation = read_annotation(cursor);
            break;
        case '[':
        {
            int count = u2(cursor);
            for (int i = 0; i < count; i++)
            {
                value->arrayValues = realloc(value->arrayValues,
                                             sizeof(void *) * (value->arrayValueCount + 1));
                value->arrayValues[value->arrayValueCount++] = read_element_value(cursor);
            }
            break;
        }
        default:
            break;
    }

    return value;
}

// Java: ClassReader.readElementValues(numElementValues, ...) - the whole annotation attribute
static void read_annotations(Cursor *cursor, LIBMATTI_ASMT_AnnotationNode ***annotations, size_t *count)
{
    int annotationCount = u2(cursor);
    for (int i = 0; i < annotationCount; i++)
    {
        LIBMATTI_ASMT_AnnotationNode *annotation = read_annotation(cursor);
        *annotations = realloc(*annotations, sizeof(void *) * (*count + 1));
        (*annotations)[(*count)++] = annotation;
    }
}

// Java: ClassReader.readCode - decode one Code attribute into the MethodNode
static void read_code(Cursor *cursor, LIBMATTI_ASMT_MethodNode *method, size_t attributeStart, size_t attributeLength)
{
    method->maxStack = u2(cursor);
    method->maxLocals = u2(cursor);
    int codeLength = (int)u4(cursor);
    size_t codeOffset = cursor->position;
    const unsigned char *code = cursor->data + codeOffset;

    method->hasCode = 1;
    method->rawCodeLength = attributeLength;
    method->rawCode = malloc(attributeLength > 0 ? attributeLength : 1);
    memcpy(method->rawCode, cursor->data + attributeStart, attributeLength);

    LabelMap labels = {0};
    InsnAt *insns = NULL;
    size_t insnCount = 0;

    size_t offset = 0;
    while (offset < (size_t)codeLength)
    {
        int opcode = code[offset];
        size_t size = instruction_size(code, offset, (size_t)codeLength);

        switch (opcode)
        {
        case LIBMATTI_ASM_NOP: case LIBMATTI_ASM_ACONST_NULL:
        case LIBMATTI_ASM_ICONST_M1: case LIBMATTI_ASM_ICONST_0: case LIBMATTI_ASM_ICONST_1:
        case LIBMATTI_ASM_ICONST_2: case LIBMATTI_ASM_ICONST_3: case LIBMATTI_ASM_ICONST_4:
        case LIBMATTI_ASM_ICONST_5: case LIBMATTI_ASM_LCONST_0: case LIBMATTI_ASM_LCONST_1:
        case LIBMATTI_ASM_FCONST_0: case LIBMATTI_ASM_FCONST_1: case LIBMATTI_ASM_FCONST_2:
        case LIBMATTI_ASM_DCONST_0: case LIBMATTI_ASM_DCONST_1:
        case LIBMATTI_ASM_IALOAD: case LIBMATTI_ASM_LALOAD: case LIBMATTI_ASM_FALOAD:
        case LIBMATTI_ASM_DALOAD: case LIBMATTI_ASM_AALOAD: case LIBMATTI_ASM_BALOAD:
        case LIBMATTI_ASM_CALOAD: case LIBMATTI_ASM_SALOAD:
        case LIBMATTI_ASM_IASTORE: case LIBMATTI_ASM_LASTORE: case LIBMATTI_ASM_FASTORE:
        case LIBMATTI_ASM_DASTORE: case LIBMATTI_ASM_AASTORE: case LIBMATTI_ASM_BASTORE:
        case LIBMATTI_ASM_CASTORE: case LIBMATTI_ASM_SASTORE:
        case LIBMATTI_ASM_POP: case LIBMATTI_ASM_POP2:
        case LIBMATTI_ASM_DUP: case LIBMATTI_ASM_DUP_X1: case LIBMATTI_ASM_DUP_X2:
        case LIBMATTI_ASM_DUP2: case LIBMATTI_ASM_DUP2_X1: case LIBMATTI_ASM_DUP2_X2:
        case LIBMATTI_ASM_SWAP:
        case LIBMATTI_ASM_IADD: case LIBMATTI_ASM_LADD: case LIBMATTI_ASM_FADD: case LIBMATTI_ASM_DADD:
        case LIBMATTI_ASM_ISUB: case LIBMATTI_ASM_LSUB: case LIBMATTI_ASM_FSUB: case LIBMATTI_ASM_DSUB:
        case LIBMATTI_ASM_IMUL: case LIBMATTI_ASM_LMUL: case LIBMATTI_ASM_FMUL: case LIBMATTI_ASM_DMUL:
        case LIBMATTI_ASM_IDIV: case LIBMATTI_ASM_LDIV: case LIBMATTI_ASM_FDIV: case LIBMATTI_ASM_DDIV:
        case LIBMATTI_ASM_IREM: case LIBMATTI_ASM_LREM: case LIBMATTI_ASM_FREM: case LIBMATTI_ASM_DREM:
        case LIBMATTI_ASM_INEG: case LIBMATTI_ASM_LNEG: case LIBMATTI_ASM_FNEG: case LIBMATTI_ASM_DNEG:
        case LIBMATTI_ASM_ISHL: case LIBMATTI_ASM_LSHL: case LIBMATTI_ASM_ISHR: case LIBMATTI_ASM_LSHR:
        case LIBMATTI_ASM_IUSHR: case LIBMATTI_ASM_LUSHR:
        case LIBMATTI_ASM_IAND: case LIBMATTI_ASM_LAND: case LIBMATTI_ASM_IOR: case LIBMATTI_ASM_LOR:
        case LIBMATTI_ASM_IXOR: case LIBMATTI_ASM_LXOR:
        case LIBMATTI_ASM_I2L: case LIBMATTI_ASM_I2F: case LIBMATTI_ASM_I2D:
        case LIBMATTI_ASM_L2I: case LIBMATTI_ASM_L2F: case LIBMATTI_ASM_L2D:
        case LIBMATTI_ASM_F2I: case LIBMATTI_ASM_F2L: case LIBMATTI_ASM_F2D:
        case LIBMATTI_ASM_D2I: case LIBMATTI_ASM_D2L: case LIBMATTI_ASM_D2F:
        case LIBMATTI_ASM_I2B: case LIBMATTI_ASM_I2C: case LIBMATTI_ASM_I2S:
        case LIBMATTI_ASM_LCMP: case LIBMATTI_ASM_FCMPL: case LIBMATTI_ASM_FCMPG:
        case LIBMATTI_ASM_DCMPL: case LIBMATTI_ASM_DCMPG:
        case LIBMATTI_ASM_IRETURN: case LIBMATTI_ASM_LRETURN: case LIBMATTI_ASM_FRETURN:
        case LIBMATTI_ASM_DRETURN: case LIBMATTI_ASM_ARETURN: case LIBMATTI_ASM_RETURN:
        case LIBMATTI_ASM_ARRAYLENGTH: case LIBMATTI_ASM_ATHROW:
        case LIBMATTI_ASM_MONITORENTER: case LIBMATTI_ASM_MONITOREXIT:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_InsnNode_New(opcode));
            break;
        case LIBMATTI_ASM_BIPUSH:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IntInsnNode_New(opcode, code[offset + 1]));
            break;
        case LIBMATTI_ASM_SIPUSH:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IntInsnNode_New(opcode, s2(code, offset + 1)));
            break;
        case LIBMATTI_ASM_NEWARRAY:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IntInsnNode_New(opcode, code[offset + 1]));
            break;
        case LIBMATTI_ASM_ILOAD: case LIBMATTI_ASM_LLOAD: case LIBMATTI_ASM_FLOAD:
        case LIBMATTI_ASM_DLOAD: case LIBMATTI_ASM_ALOAD:
        case LIBMATTI_ASM_ISTORE: case LIBMATTI_ASM_LSTORE: case LIBMATTI_ASM_FSTORE:
        case LIBMATTI_ASM_DSTORE: case LIBMATTI_ASM_ASTORE:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_VarInsnNode_New(opcode, code[offset + 1]));
            break;
        case LIBMATTI_ASM_IINC:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IincInsnNode_New(code[offset + 1],
                                                                                      (signed char)code[offset + 2]));
            break;
        case LIBMATTI_ASM_WIDE:
        {
            int realOpcode = code[offset + 1];
            if (realOpcode == LIBMATTI_ASM_IINC)
                add_insn(&insns, &insnCount, (int)offset,
                         (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_IincInsnNode_New(
                             s2(code, offset + 2), s2(code, offset + 4)));
            else
                add_insn(&insns, &insnCount, (int)offset,
                         (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_VarInsnNode_New(realOpcode,
                                                                                          s2(code, offset + 2)));
            break;
        }
        case LIBMATTI_ASM_NEW: case LIBMATTI_ASM_ANEWARRAY: case LIBMATTI_ASM_CHECKCAST:
        case LIBMATTI_ASM_INSTANCEOF:
        {
            int index = s2(code, offset + 1);
            const char *desc = index == 0 ? NULL : LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, index);
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_TypeInsnNode_New(opcode, desc));
            break;
        }
        case LIBMATTI_ASM_GETSTATIC: case LIBMATTI_ASM_PUTSTATIC:
        case LIBMATTI_ASM_GETFIELD: case LIBMATTI_ASM_PUTFIELD:
        {
            int index = s2(code, offset + 1);
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_FieldInsnNode_New(
                         opcode, member_owner(cursor, index), member_name(cursor, index),
                         member_descriptor(cursor, index)));
            break;
        }
        case LIBMATTI_ASM_INVOKEVIRTUAL: case LIBMATTI_ASM_INVOKESPECIAL:
        case LIBMATTI_ASM_INVOKESTATIC: case LIBMATTI_ASM_INVOKEINTERFACE:
        {
            int index = s2(code, offset + 1);
            int itf = opcode == LIBMATTI_ASM_INVOKEINTERFACE ||
                      (index > 0 && (size_t)index < cursor->pool->count &&
                       cursor->pool->entries[index].tag == CP_INTERFACE_METHODREF);
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_MethodInsnNode_New(
                         opcode, member_owner(cursor, index), member_name(cursor, index),
                         member_descriptor(cursor, index), itf));
            break;
        }
        case LIBMATTI_ASM_INVOKEDYNAMIC:
        {
            int index = s2(code, offset + 1);
            const char *name = index > 0 && (size_t)index < cursor->pool->count
                                   ? utf8(cursor, cursor->pool->entries[cursor->pool->entries[index].index2].index1)
                                   : NULL;
            const char *desc = index > 0 && (size_t)index < cursor->pool->count
                                   ? utf8(cursor, cursor->pool->entries[cursor->pool->entries[index].index2].index2)
                                   : NULL;
            LIBMATTI_ASMT_InvokeDynamicInsnNode *dynamic = calloc(1, sizeof(*dynamic));
            dynamic->base.type = LIBMATTI_ASMT_INVOKE_DYNAMIC_INSN;
            dynamic->base.opcode = opcode;
            dynamic->name = strdup(name != NULL ? name : "");
            dynamic->desc = strdup(desc != NULL ? desc : "()V");
            dynamic->bsmIndex = index;
            add_insn(&insns, &insnCount, (int)offset, (LIBMATTI_ASMT_AbstractInsnNode *)dynamic);
            break;
        }
        case LIBMATTI_ASM_IFEQ: case LIBMATTI_ASM_IFNE: case LIBMATTI_ASM_IFLT:
        case LIBMATTI_ASM_IFGE: case LIBMATTI_ASM_IFGT: case LIBMATTI_ASM_IFLE:
        case LIBMATTI_ASM_IF_ICMPEQ: case LIBMATTI_ASM_IF_ICMPNE: case LIBMATTI_ASM_IF_ICMPLT:
        case LIBMATTI_ASM_IF_ICMPGE: case LIBMATTI_ASM_IF_ICMPGT: case LIBMATTI_ASM_IF_ICMPLE:
        case LIBMATTI_ASM_IF_ACMPEQ: case LIBMATTI_ASM_IF_ACMPNE:
        case LIBMATTI_ASM_GOTO: case LIBMATTI_ASM_JSR:
        case LIBMATTI_ASM_IFNULL: case LIBMATTI_ASM_IFNONNULL:
        case LIBMATTI_ASM_GOTO_W: case LIBMATTI_ASM_JSR_W:
        {
            int target = opcode == LIBMATTI_ASM_GOTO_W || opcode == LIBMATTI_ASM_JSR_W
                             ? (int)offset + s4(code, offset + 1)
                             : (int)offset + s2(code, offset + 1);
            LIBMATTI_ASMT_JumpInsnNode *jump = calloc(1, sizeof(*jump));
            jump->base.type = LIBMATTI_ASMT_JUMP_INSN;
            jump->base.opcode = opcode;
            jump->label = label_node_for(&labels, label_for(&labels, target));
            add_insn(&insns, &insnCount, (int)offset, (LIBMATTI_ASMT_AbstractInsnNode *)jump);
            break;
        }
        case LIBMATTI_ASM_LDC: case LIBMATTI_ASM_LDC_W: case LIBMATTI_ASM_LDC2_W:
        {
            int index = opcode == LIBMATTI_ASM_LDC ? code[offset + 1] : s2(code, offset + 1);
            size_t constantSize = 1;
            LIBMATTI_ASM_Object constant = ldap_constant(cursor, index, &constantSize);
            LIBMATTI_ASMT_LdcInsnNode *ldc = calloc(1, sizeof(*ldc));
            ldc->base.type = LIBMATTI_ASMT_LDC_INSN;
            ldc->base.opcode = opcode;
            ldc->cst = constant;
            add_insn(&insns, &insnCount, (int)offset, (LIBMATTI_ASMT_AbstractInsnNode *)ldc);
            break;
        }
        case LIBMATTI_ASM_TABLESWITCH:
        {
            size_t cursorPos = offset + 1;
            while ((cursorPos & 3) != 0) cursorPos++;
            int dflt = (int)offset + s4(code, cursorPos);
            int low = s4(code, cursorPos + 4);
            int high = s4(code, cursorPos + 8);
            LIBMATTI_ASMT_TableSwitchInsnNode *node = calloc(1, sizeof(*node));
            node->base.type = LIBMATTI_ASMT_TABLE_SWITCH_INSN;
            node->base.opcode = opcode;
            node->min = low;
            node->max = high;
            node->dflt = label_node_for(&labels, label_for(&labels, dflt));
            node->labelCount = (size_t)(high - low + 1);
            node->labels = malloc(sizeof(*node->labels) * node->labelCount);
            for (size_t i = 0; i < node->labelCount; i++)
                node->labels[i] = label_node_for(&labels,
                                                 label_for(&labels, (int)offset + s4(code, cursorPos + 12 + i * 4)));
            add_insn(&insns, &insnCount, (int)offset, (LIBMATTI_ASMT_AbstractInsnNode *)node);
            break;
        }
        case LIBMATTI_ASM_LOOKUPSWITCH:
        {
            size_t cursorPos = offset + 1;
            while ((cursorPos & 3) != 0) cursorPos++;
            int dflt = (int)offset + s4(code, cursorPos);
            int count = s4(code, cursorPos + 4);
            LIBMATTI_ASMT_LookupSwitchInsnNode *node = calloc(1, sizeof(*node));
            node->base.type = LIBMATTI_ASMT_LOOKUP_SWITCH_INSN;
            node->base.opcode = opcode;
            node->dflt = label_node_for(&labels, label_for(&labels, dflt));
            node->labelCount = (size_t)count;
            node->keys = malloc(sizeof(*node->keys) * node->labelCount);
            node->labels = malloc(sizeof(*node->labels) * node->labelCount);
            for (size_t i = 0; i < node->labelCount; i++)
            {
                node->keys[i] = s4(code, cursorPos + 8 + i * 8);
                node->labels[i] = label_node_for(&labels,
                                                 label_for(&labels, (int)offset + s4(code, cursorPos + 8 + i * 8 + 4)));
            }
            add_insn(&insns, &insnCount, (int)offset, (LIBMATTI_ASMT_AbstractInsnNode *)node);
            break;
        }
        case LIBMATTI_ASM_MULTIANEWARRAY:
        {
            int index = s2(code, offset + 1);
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_MultiANewArrayInsnNode_New(
                         utf8(cursor, index), code[offset + 3]));
            break;
        }
        default:
            add_insn(&insns, &insnCount, (int)offset,
                     (LIBMATTI_ASMT_AbstractInsnNode *)LIBMATTI_ASMT_InsnNode_New(opcode));
            break;
        }

        offset += size;
    }

    // exception table
    cursor->position = codeOffset + (size_t)codeLength;
    int tryCatchCount = u2(cursor);
    for (int i = 0; i < tryCatchCount; i++)
    {
        int start = u2(cursor);
        int end = u2(cursor);
        int handler = u2(cursor);
        int type = u2(cursor);

        method->tryCatchBlocks = realloc(method->tryCatchBlocks,
                                         sizeof(*method->tryCatchBlocks) * (method->tryCatchBlockCount + 1));
        LIBMATTI_ASMT_TryCatchBlockNode *block = &method->tryCatchBlocks[method->tryCatchBlockCount++];
        block->start = label_node_for(&labels, label_for(&labels, start));
        block->end = label_node_for(&labels, label_for(&labels, end));
        block->handler = label_node_for(&labels, label_for(&labels, handler));
        block->type = type == 0 ? NULL : strdup(LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, type));
    }

    // code attributes: line numbers and stack map frames
    int codeAttributeCount = u2(cursor);
    LIBMATTI_ASM_Object *frameLocals = NULL;
    size_t frameLocalCount = 0;

    for (int i = 0; i < codeAttributeCount; i++)
    {
        int nameIndex = u2(cursor);
        int length = (int)u4(cursor);
        size_t attributeStart = cursor->position;
        const char *name = utf8(cursor, nameIndex);

        if (name != NULL && strcmp(name, "LineNumberTable") == 0 && !(cursor->flags & LIBMATTI_ASM_SKIP_DEBUG))
        {
            int count = u2(cursor);
            for (int l = 0; l < count; l++)
            {
                int startPc = u2(cursor);
                int line = u2(cursor);
                LIBMATTI_ASMT_LineNumberNode *lineNode = calloc(1, sizeof(*lineNode));
                lineNode->base.type = LIBMATTI_ASMT_LINE_NUMBER;
                lineNode->base.opcode = -1;
                lineNode->line = line;
                lineNode->start = label_for(&labels, startPc);
                add_insn(&insns, &insnCount, startPc, (LIBMATTI_ASMT_AbstractInsnNode *)lineNode);
            }
        }
        else if (name != NULL && strcmp(name, "StackMapTable") == 0 && !(cursor->flags & LIBMATTI_ASM_SKIP_FRAMES))
        {
            int count = u2(cursor);
            int previousOffset = -1;

            for (int f = 0; f < count; f++)
            {
                int frameType = u1(cursor);
                int offsetDelta;

                if (frameType < 64)
                {
                    offsetDelta = frameType;
                }
                else if (frameType < 128)
                {
                    offsetDelta = frameType - 64;
                }
                else if (frameType == 247)
                {
                    offsetDelta = u2(cursor);
                }
                else if (frameType >= 248 && frameType <= 251)
                {
                    offsetDelta = u2(cursor);
                }
                else if (frameType >= 252 && frameType <= 254)
                {
                    offsetDelta = u2(cursor);
                }
                else
                {
                    offsetDelta = u2(cursor);
                }

                int frameOffset = offsetDelta + previousOffset + 1;
                previousOffset = frameOffset;

                // Java: EXPAND_FRAMES produces full frames; apply the delta to the
                // running locals array.
                if (frameType >= 0 && frameType < 64)
                {
                    // same frame
                }
                else if (frameType >= 64 && frameType < 128)
                {
                    // same locals, one stack item: locals unchanged
                }
                else if (frameType == 247)
                {
                    // same locals, one stack item (extended)
                }
                else if (frameType >= 248 && frameType <= 250)
                {
                    // chop
                    int chop = 251 - frameType;
                    while (chop-- > 0 && frameLocalCount > 0)
                        LIBMATTI_ASM_Object_Free(&frameLocals[--frameLocalCount]);
                }
                else if (frameType == 251)
                {
                    // same frame (extended)
                }
                else if (frameType >= 252 && frameType <= 254)
                {
                    // append
                    int append = frameType - 251;
                    for (int a = 0; a < append; a++)
                    {
                        int tag = u1(cursor);
                        LIBMATTI_ASM_Object value = {0};
                        if (tag == 7)
                        {
                            value = LIBMATTI_ASM_Object_OfString(LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool,
                                                                                                    u2(cursor)));
                        }
                        else if (tag == 8)
                        {
                            value = LIBMATTI_ASM_Object_OfLabel(label_for(&labels, u2(cursor)));
                        }
                        else
                        {
                            value = LIBMATTI_ASM_Object_OfInt(tag);
                        }
                        frameLocals = realloc(frameLocals, sizeof(*frameLocals) * (frameLocalCount + 1));
                        frameLocals[frameLocalCount++] = value;
                    }
                }
                else if (frameType == 255)
                {
                    // full frame
                    for (size_t l = 0; l < frameLocalCount; l++) LIBMATTI_ASM_Object_Free(&frameLocals[l]);
                    frameLocalCount = 0;

                    int localCount = u2(cursor);
                    for (int l = 0; l < localCount; l++)
                    {
                        int tag = u1(cursor);
                        LIBMATTI_ASM_Object value = {0};
                        if (tag == 7) value = LIBMATTI_ASM_Object_OfString(
                                LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, u2(cursor)));
                        else if (tag == 8) value = LIBMATTI_ASM_Object_OfLabel(label_for(&labels, u2(cursor)));
                        else value = LIBMATTI_ASM_Object_OfInt(tag);
                        frameLocals = realloc(frameLocals, sizeof(*frameLocals) * (frameLocalCount + 1));
                        frameLocals[frameLocalCount++] = value;
                    }
                }

                // stack items
                int stackCount = 0;
                LIBMATTI_ASM_Object *stack = NULL;
                if (frameType >= 64 && frameType < 128)
                {
                    stackCount = 1;
                    stack = malloc(sizeof(*stack));
                    int tag = u1(cursor);
                    if (tag == 7) stack[0] = LIBMATTI_ASM_Object_OfString(
                            LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, u2(cursor)));
                    else if (tag == 8) stack[0] = LIBMATTI_ASM_Object_OfLabel(label_for(&labels, u2(cursor)));
                    else stack[0] = LIBMATTI_ASM_Object_OfInt(tag);
                }
                else if (frameType == 247)
                {
                    stackCount = 1;
                    stack = malloc(sizeof(*stack));
                    int tag = u1(cursor);
                    if (tag == 7) stack[0] = LIBMATTI_ASM_Object_OfString(
                            LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, u2(cursor)));
                    else if (tag == 8) stack[0] = LIBMATTI_ASM_Object_OfLabel(label_for(&labels, u2(cursor)));
                    else stack[0] = LIBMATTI_ASM_Object_OfInt(tag);
                }
                else if (frameType == 255)
                {
                    stackCount = u2(cursor);
                    stack = malloc(sizeof(*stack) * (stackCount > 0 ? stackCount : 1));
                    for (int s = 0; s < stackCount; s++)
                    {
                        int tag = u1(cursor);
                        if (tag == 7) stack[s] = LIBMATTI_ASM_Object_OfString(
                                LIBMATTI_ASM_ConstantPool_ClassName(cursor->pool, u2(cursor)));
                        else if (tag == 8) stack[s] = LIBMATTI_ASM_Object_OfLabel(label_for(&labels, u2(cursor)));
                        else stack[s] = LIBMATTI_ASM_Object_OfInt(tag);
                    }
                }

                LIBMATTI_ASMT_FrameNode *frame = calloc(1, sizeof(*frame));
                frame->base.type = LIBMATTI_ASMT_FRAME;
                frame->base.opcode = -1;
                frame->frameType = LIBMATTI_ASM_F_NEW;
                frame->localCount = frameLocalCount;
                frame->local = malloc(sizeof(*frame->local) * (frameLocalCount > 0 ? frameLocalCount : 1));
                for (size_t l = 0; l < frameLocalCount; l++) frame->local[l] = object_clone(&frameLocals[l]);
                frame->stackCount = (size_t)stackCount;
                frame->stack = stack;

                add_insn(&insns, &insnCount, frameOffset, (LIBMATTI_ASMT_AbstractInsnNode *)frame);
            }
        }

        cursor->position = attributeStart + (size_t)length;
    }

    // Java: ClassReader emits, per bytecode offset, the label first, then line
    // numbers, then stack map frames, then the instruction.
    size_t entryCount = insnCount + labels.count;
    InsnAt *entries = calloc(entryCount > 0 ? entryCount : 1, sizeof(InsnAt));
    size_t entryIndex = 0;
    int *entryRank = calloc(entryCount > 0 ? entryCount : 1, sizeof(int));

    for (size_t l = 0; l < labels.count; l++)
    {
        LabelEntry *entry = &labels.entries[l];
        if (entry->labelNode == NULL) entry->labelNode = LIBMATTI_ASMT_LabelNode_New(entry->label);
        entries[entryIndex].offset = entry->label->offset;
        entries[entryIndex].node = (LIBMATTI_ASMT_AbstractInsnNode *)entry->labelNode;
        entryRank[entryIndex] = 0;
        entryIndex++;
    }

    for (size_t i = 0; i < insnCount; i++)
    {
        int rank = 3;
        if (insns[i].node->type == LIBMATTI_ASMT_LINE_NUMBER) rank = 1;
        else if (insns[i].node->type == LIBMATTI_ASMT_FRAME) rank = 2;
        entries[entryIndex].offset = insns[i].offset;
        entries[entryIndex].node = insns[i].node;
        entryRank[entryIndex] = rank;
        entryIndex++;
    }

    // stable insertion sort (equal offsets keep label < line < frame < insn)
    for (size_t i = 1; i < entryIndex; i++)
    {
        for (size_t j = i; j > 0; j--)
        {
            int before = entries[j - 1].offset < entries[j].offset ||
                         (entries[j - 1].offset == entries[j].offset && entryRank[j - 1] <= entryRank[j]);
            if (before) break;
            InsnAt swapInsn = entries[j - 1];
            entries[j - 1] = entries[j];
            entries[j] = swapInsn;
            int swapRank = entryRank[j - 1];
            entryRank[j - 1] = entryRank[j];
            entryRank[j] = swapRank;
        }
    }

    for (size_t i = 0; i < entryIndex; i++) LIBMATTI_ASM_InsnList_Add(&method->instructions, entries[i].node);

    // Java: a freshly read instruction list is not "modified"
    method->instructions.modified = 0;

    free(entryRank);
    free(entries);
    free(insns);
    for (size_t l = 0; l < frameLocalCount; l++) LIBMATTI_ASM_Object_Free(&frameLocals[l]);
    free(frameLocals);
    // the Label objects are owned by the LabelNodes created for them
    free(labels.entries);

    cursor->position = attributeStart + attributeLength;
}

LIBMATTI_ASM_ClassReader *LIBMATTI_ASM_ClassReader_New(const unsigned char *data, size_t length, int flags)
{
    LIBMATTI_ASM_ClassReader *reader = calloc(1, sizeof(LIBMATTI_ASM_ClassReader));
    reader->data = data;
    reader->length = length;
    reader->flags = flags;
    return reader;
}

void LIBMATTI_ASM_ClassReader_Free(LIBMATTI_ASM_ClassReader *reader)
{
    free(reader);
}

void LIBMATTI_ASM_ClassReader_Accept(LIBMATTI_ASM_ClassReader *reader, LIBMATTI_ASMT_ClassNode *node)
{
    Cursor cursor = {reader->data, reader->length, 0, reader->flags, node->pool};

    if ((unsigned)u4(&cursor) != 0xCAFEBABEu)
    {
        // Java: throws IllegalArgumentException("Illegal class file")
        return;
    }

    int minor = u2(&cursor);
    int major = u2(&cursor);
    node->version = (minor << 16) | major;

    read_constant_pool(&cursor);

    node->access = u2(&cursor);
    int thisClass = u2(&cursor);
    int superClass = u2(&cursor);

    const char *name = LIBMATTI_ASM_ConstantPool_ClassName(node->pool, thisClass);
    node->name = name != NULL ? strdup(name) : NULL;

    if (superClass != 0)
    {
        const char *superName = LIBMATTI_ASM_ConstantPool_ClassName(node->pool, superClass);
        node->superName = superName != NULL ? strdup(superName) : NULL;
    }

    int interfaceCount = u2(&cursor);
    if (interfaceCount > 0)
    {
        node->interfaces = malloc(sizeof(*node->interfaces) * interfaceCount);
        node->interfaceCount = (size_t)interfaceCount;
        for (int i = 0; i < interfaceCount; i++)
            node->interfaces[i] = strdup(LIBMATTI_ASM_ConstantPool_ClassName(node->pool, u2(&cursor)));
    }

    int fieldCount = u2(&cursor);
    for (int i = 0; i < fieldCount; i++)
    {
        int access = u2(&cursor);
        int nameIndex = u2(&cursor);
        int descriptorIndex = u2(&cursor);
        int attributeCount = u2(&cursor);

        LIBMATTI_ASMT_FieldNode *field = LIBMATTI_ASMT_FieldNode_New(access, utf8(&cursor, nameIndex),
                                                                     utf8(&cursor, descriptorIndex), NULL, NULL);

        for (int a = 0; a < attributeCount; a++)
        {
            int attributeNameIndex = u2(&cursor);
            int length = (int)u4(&cursor);
            size_t attributeStart = cursor.position;
            const char *attributeName = utf8(&cursor, attributeNameIndex);

            if (attributeName != NULL && strcmp(attributeName, "Signature") == 0)
            {
                field->signature = strdup(utf8(&cursor, u2(&cursor)));
            }
            else if (attributeName != NULL && strcmp(attributeName, "ConstantValue") == 0)
            {
                size_t constantSize = 1;
                field->value = ldap_constant(&cursor, u2(&cursor), &constantSize);
            }
            else if (attributeName != NULL && strcmp(attributeName, "RuntimeVisibleAnnotations") == 0)
            {
                // Java: ClassReader.readElementValues into annotationVisitor.visitAnnotation(...)
                read_annotations(&cursor, &field->visibleAnnotations, &field->visibleAnnotationCount);
            }
            else if (attributeName != NULL && strcmp(attributeName, "RuntimeInvisibleAnnotations") == 0)
            {
                read_annotations(&cursor, &field->invisibleAnnotations, &field->invisibleAnnotationCount);
            }
            else
            {
                field->attrs = realloc(field->attrs, sizeof(*field->attrs) * (field->attrCount + 1));
                field->attrs[field->attrCount++] = LIBMATTI_ASM_Attribute_New(
                    attributeName != NULL ? attributeName : "", cursor.data + attributeStart, (size_t)length);
            }

            cursor.position = attributeStart + (size_t)length;
        }

        node->fields = realloc(node->fields, sizeof(*node->fields) * (node->fieldCount + 1));
        node->fields[node->fieldCount++] = field;
    }

    int methodCount = u2(&cursor);
    for (int i = 0; i < methodCount; i++)
    {
        int access = u2(&cursor);
        int nameIndex = u2(&cursor);
        int descriptorIndex = u2(&cursor);
        int attributeCount = u2(&cursor);

        LIBMATTI_ASMT_MethodNode *method = LIBMATTI_ASMT_MethodNode_New(access, utf8(&cursor, nameIndex),
                                                                        utf8(&cursor, descriptorIndex), NULL, NULL, 0);

        for (int a = 0; a < attributeCount; a++)
        {
            int attributeNameIndex = u2(&cursor);
            int length = (int)u4(&cursor);
            size_t attributeStart = cursor.position;
            const char *attributeName = utf8(&cursor, attributeNameIndex);

            if (attributeName != NULL && strcmp(attributeName, "Signature") == 0)
            {
                method->signature = strdup(utf8(&cursor, u2(&cursor)));
            }
            else if (attributeName != NULL && strcmp(attributeName, "Exceptions") == 0)
            {
                int exceptionCount = u2(&cursor);
                if (exceptionCount > 0)
                {
                    method->exceptions = malloc(sizeof(*method->exceptions) * (size_t)exceptionCount);
                    method->exceptionCount = (size_t)exceptionCount;
                    for (int e = 0; e < exceptionCount; e++)
                        method->exceptions[e] = strdup(LIBMATTI_ASM_ConstantPool_ClassName(node->pool, u2(&cursor)));
                }
            }
            else if (attributeName != NULL && strcmp(attributeName, "Code") == 0)
            {
                read_code(&cursor, method, attributeStart, (size_t)length);
            }
            else if (attributeName != NULL && strcmp(attributeName, "RuntimeVisibleAnnotations") == 0)
            {
                read_annotations(&cursor, &method->visibleAnnotations, &method->visibleAnnotationCount);
            }
            else if (attributeName != NULL && strcmp(attributeName, "RuntimeInvisibleAnnotations") == 0)
            {
                read_annotations(&cursor, &method->invisibleAnnotations, &method->invisibleAnnotationCount);
            }
            else
            {
                method->attrs = realloc(method->attrs, sizeof(*method->attrs) * (method->attrCount + 1));
                method->attrs[method->attrCount++] = LIBMATTI_ASM_Attribute_New(
                    attributeName != NULL ? attributeName : "", cursor.data + attributeStart, (size_t)length);
            }

            cursor.position = attributeStart + (size_t)length;
        }

        node->methods = realloc(node->methods, sizeof(*node->methods) * (node->methodCount + 1));
        node->methods[node->methodCount++] = method;
    }

    int attributeCount = u2(&cursor);
    for (int a = 0; a < attributeCount; a++)
    {
        int attributeNameIndex = u2(&cursor);
        int length = (int)u4(&cursor);
        size_t attributeStart = cursor.position;
        const char *attributeName = utf8(&cursor, attributeNameIndex);

        if (attributeName != NULL && strcmp(attributeName, "SourceFile") == 0)
            node->sourceFile = strdup(utf8(&cursor, u2(&cursor)));
        else if (attributeName != NULL && strcmp(attributeName, "RuntimeVisibleAnnotations") == 0)
            read_annotations(&cursor, &node->visibleAnnotations, &node->visibleAnnotationCount);
        else if (attributeName != NULL && strcmp(attributeName, "RuntimeInvisibleAnnotations") == 0)
            read_annotations(&cursor, &node->invisibleAnnotations, &node->invisibleAnnotationCount);
        else
        {
            node->attrs = realloc(node->attrs, sizeof(*node->attrs) * (node->attrCount + 1));
            node->attrs[node->attrCount++] = LIBMATTI_ASM_Attribute_New(
                attributeName != NULL ? attributeName : "", cursor.data + attributeStart, (size_t)length);
        }

        cursor.position = attributeStart + (size_t)length;
    }
}
