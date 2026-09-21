// Port of org.objectweb.asm.commons.GeneratorAdapter.

#include "libmatti/org/objectweb/asm/commons/GeneratorAdapter.h"

#include "libmatti/org/objectweb/asm/Object.h"
#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Type OBJECT_TYPE
static const char *OBJECT_DESCRIPTOR = "Ljava/lang/Object;";
static const char *NUMBER_DESCRIPTOR = "Ljava/lang/Number;";
static const char *BOOLEAN_DESCRIPTOR = "Ljava/lang/Boolean;";
static const char *CHARACTER_DESCRIPTOR = "Ljava/lang/Character;";

// Java: private static final Method CHAR_VALUE, BOOLEAN_VALUE, DOUBLE_VALUE, ... of java/lang/Number
static const char *NUMBER_INT_VALUE_DESCRIPTOR = "()I";
static const char *NUMBER_LONG_VALUE_DESCRIPTOR = "()J";
static const char *NUMBER_FLOAT_VALUE_DESCRIPTOR = "()F";
static const char *NUMBER_DOUBLE_VALUE_DESCRIPTOR = "()D";
static const char *CHAR_CHAR_VALUE_DESCRIPTOR = "()C";
static const char *BOOLEAN_BOOLEAN_VALUE_DESCRIPTOR = "()Z";

// Java: the internal name of a Type (for arrays getInternalName() is the descriptor)
static char *internal_name_of(const LIBMATTI_ASM_Type *type)
{
    return LIBMATTI_ASM_Type_GetInternalName(type);
}

// Java: mv.visitInsn(opcode)
static void emit_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList, (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_InsnNode_New(opcode));
}

// Java: mv.visitIntInsn(opcode, operand)
static void emit_int_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode, int operand)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_IntInsnNode_New(opcode, operand));
}

// Java: mv.visitTypeInsn(opcode, type)
static void emit_type_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode, const char *type)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_TypeInsnNode_New(opcode, type));
}

// Java: mv.visitFieldInsn(opcode, owner, name, descriptor)
static void emit_field_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode, const char *owner,
                            const char *name, const char *descriptor)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_FieldInsnNode_New(
                                  opcode, owner, name, descriptor));
}

// Java: mv.visitMethodInsn(opcode, owner, name, descriptor, isInterface)
static void emit_method_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode, const char *owner,
                             const char *name, const char *descriptor, int isInterface)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_MethodInsnNode_New(
                                  opcode, owner, name, descriptor, isInterface));
}

// Java: mv.visitLdcInsn(value)
static void emit_ldc(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, const LIBMATTI_ASM_Object *value)
{
    LIBMATTI_ASM_InsnList_Add(adapter->insnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_LdcInsnNode_New(value));
}

// Java: private void invokeInsn(int opcode, Type type, Method method, boolean isInterface)
static void invoke_insn(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int opcode, const LIBMATTI_ASM_Type *type,
                        const LIBMATTI_ASM_COMMONS_Method *method, int isInterface)
{
    // Java: String owner = type.getSort() == Type.ARRAY ? type.getDescriptor() : type.getInternalName();
    char *owner = internal_name_of(type);
    emit_method_insn(adapter, opcode, owner, LIBMATTI_ASM_COMMONS_Method_GetName(method),
                     LIBMATTI_ASM_COMMONS_Method_GetDescriptor(method), isInterface);
    free(owner);
}

// Java: protected GeneratorAdapter(int api, MethodVisitor methodVisitor, int access, String name, String descriptor)
LIBMATTI_ASM_COMMONS_GeneratorAdapter *LIBMATTI_ASM_COMMONS_GeneratorAdapter_New(
    int api, int access, const char *name, const char *descriptor, LIBMATTI_ASMT_InsnList *insnList)
{
    (void) api;

    LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter = calloc(1, sizeof(LIBMATTI_ASM_COMMONS_GeneratorAdapter));
    adapter->insnList = insnList;
    adapter->access = access;
    adapter->name = name != NULL ? strdup(name) : NULL;
    adapter->descriptor = descriptor != NULL ? strdup(descriptor) : NULL;

    // Java: LocalVariablesSorter computes the first local slot from the static flag
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetMethodType(descriptor);
    size_t argumentCount = 0;
    LIBMATTI_ASM_Type **argumentTypes = LIBMATTI_ASM_Type_GetArgumentTypes(methodType, &argumentCount);

    adapter->firstLocal = 0;
    for (size_t i = 0; i < argumentCount; i++)
    {
        adapter->firstLocal += LIBMATTI_ASM_Type_GetSize(argumentTypes[i]);
        LIBMATTI_ASM_Type_Free(argumentTypes[i]);
    }
    free(argumentTypes);
    LIBMATTI_ASM_Type_Free(methodType);

    adapter->nextLocal = adapter->firstLocal;
    return adapter;
}

void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Free(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    if (adapter == NULL) return;
    free(adapter->name);
    free(adapter->descriptor);
    free(adapter);
}

// Java: public int newLocal(Type type)
int LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewLocal(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                   const LIBMATTI_ASM_Type *type)
{
    // Java: LocalVariablesSorter.newLocal - the port hands out the next free slot
    int local = adapter->nextLocal;
    adapter->nextLocal += LIBMATTI_ASM_Type_GetSize(type);
    return local;
}

// Java: public void push(int value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int value)
{
    if (value >= -1 && value <= 5)
    {
        emit_insn(adapter, LIBMATTI_ASM_ICONST_0 + value);
    }
    else if (value >= -128 && value <= 127)
    {
        emit_int_insn(adapter, LIBMATTI_ASM_BIPUSH, value);
    }
    else if (value >= -32768 && value <= 32767)
    {
        emit_int_insn(adapter, LIBMATTI_ASM_SIPUSH, value);
    }
    else
    {
        LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfInt(value);
        emit_ldc(adapter, &constant);
    }
}

// Java: public void push(boolean value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushBool(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int value)
{
    emit_insn(adapter, value ? LIBMATTI_ASM_ICONST_1 : LIBMATTI_ASM_ICONST_0);
}

// Java: public void push(long value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushLong(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, long long value)
{
    if (value == 0LL || value == 1LL)
    {
        emit_insn(adapter, LIBMATTI_ASM_LCONST_0 + (int) value);
        return;
    }

    LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfLong(value);
    emit_ldc(adapter, &constant);
}

// Java: public void push(float value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushFloat(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, float value)
{
    if (value == 0.0f || value == 1.0f || value == 2.0f)
    {
        emit_insn(adapter, LIBMATTI_ASM_FCONST_0 + (int) value);
        return;
    }

    LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfFloat(value);
    emit_ldc(adapter, &constant);
}

// Java: public void push(double value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushDouble(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, double value)
{
    if (value == 0.0 || value == 1.0)
    {
        emit_insn(adapter, LIBMATTI_ASM_DCONST_0 + (int) value);
        return;
    }

    LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfDouble(value);
    emit_ldc(adapter, &constant);
}

// Java: public void push(String value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const char *value)
{
    if (value == NULL)
    {
        emit_insn(adapter, LIBMATTI_ASM_ACONST_NULL);
        return;
    }

    LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfString(value);
    emit_ldc(adapter, &constant);
}

// Java: public void push(Type value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushType(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *value)
{
    if (value == NULL)
    {
        emit_insn(adapter, LIBMATTI_ASM_ACONST_NULL);
        return;
    }

    // Java: the primitive class literals are read from the wrapper TYPE fields
    switch (LIBMATTI_ASM_Type_GetSort(value))
    {
        case LIBMATTI_ASM_TYPE_BOOLEAN:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Boolean", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_CHAR:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Character", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_BYTE:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Byte", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_SHORT:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Short", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_INT:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Integer", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_FLOAT:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Float", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_LONG:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Long", "TYPE", "Ljava/lang/Class;");
            return;
        case LIBMATTI_ASM_TYPE_DOUBLE:
            emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, "java/lang/Double", "TYPE", "Ljava/lang/Class;");
            return;
        default:
        {
            LIBMATTI_ASM_Object constant = LIBMATTI_ASM_Object_OfType(value);
            emit_ldc(adapter, &constant);
            return;
        }
    }
}

// Java: public void dup()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    emit_insn(adapter, LIBMATTI_ASM_DUP);
}

// Java: public void swap()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Swap(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    emit_insn(adapter, LIBMATTI_ASM_SWAP);
}

// Java: public void arrayLength()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayLength(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    emit_insn(adapter, LIBMATTI_ASM_ARRAYLENGTH);
}

// Java: public void arrayStore(Type type) { mv.visitInsn(type.getOpcode(Opcodes.IASTORE)); }
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayStore(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const LIBMATTI_ASM_Type *type)
{
    emit_insn(adapter, LIBMATTI_ASM_Type_GetOpcode(type, LIBMATTI_ASM_IASTORE));
}

// Java: public void math(int op, Type type) { mv.visitInsn(type.getOpcode(op)); }
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Math(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int op,
                                                const LIBMATTI_ASM_Type *type)
{
    emit_insn(adapter, LIBMATTI_ASM_Type_GetOpcode(type, op));
}

// Java: public void not() { mv.visitInsn(Opcodes.ICONST_1); mv.visitInsn(Opcodes.IXOR); }
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Not(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    emit_insn(adapter, LIBMATTI_ASM_ICONST_1);
    emit_insn(adapter, LIBMATTI_ASM_IXOR);
}

// Java: public void checkCast(Type type) { if (!type.equals(OBJECT_TYPE)) mv.visitTypeInsn(Opcodes.CHECKCAST, type.getInternalName()); }
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_CheckCast(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *type)
{
    if (strcmp(LIBMATTI_ASM_Type_GetDescriptor(type), OBJECT_DESCRIPTOR) == 0) return;

    char *internalName = internal_name_of(type);
    emit_type_insn(adapter, LIBMATTI_ASM_CHECKCAST, internalName);
    free(internalName);
}

// Java: public void instanceOf(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InstanceOf(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const LIBMATTI_ASM_Type *type)
{
    char *internalName = internal_name_of(type);
    emit_type_insn(adapter, LIBMATTI_ASM_INSTANCEOF, internalName);
    free(internalName);
}

// Java: public void newInstance(Type type) { mv.visitTypeInsn(Opcodes.NEW, type.getInternalName()); }
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewInstance(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                       const LIBMATTI_ASM_Type *type)
{
    char *internalName = internal_name_of(type);
    emit_type_insn(adapter, LIBMATTI_ASM_NEW, internalName);
    free(internalName);
}

// Java: public void newArray(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewArray(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *type)
{
    int arrayType = 0;
    switch (LIBMATTI_ASM_Type_GetSort(type))
    {
        case LIBMATTI_ASM_TYPE_BOOLEAN: arrayType = LIBMATTI_ASM_T_BOOLEAN; break;
        case LIBMATTI_ASM_TYPE_CHAR: arrayType = LIBMATTI_ASM_T_CHAR; break;
        case LIBMATTI_ASM_TYPE_BYTE: arrayType = LIBMATTI_ASM_T_BYTE; break;
        case LIBMATTI_ASM_TYPE_SHORT: arrayType = LIBMATTI_ASM_T_SHORT; break;
        case LIBMATTI_ASM_TYPE_INT: arrayType = LIBMATTI_ASM_T_INT; break;
        case LIBMATTI_ASM_TYPE_FLOAT: arrayType = LIBMATTI_ASM_T_FLOAT; break;
        case LIBMATTI_ASM_TYPE_LONG: arrayType = LIBMATTI_ASM_T_LONG; break;
        case LIBMATTI_ASM_TYPE_DOUBLE: arrayType = LIBMATTI_ASM_T_DOUBLE; break;
        default:
        {
            char *internalName = internal_name_of(type);
            emit_type_insn(adapter, LIBMATTI_ASM_ANEWARRAY, internalName);
            free(internalName);
            return;
        }
    }

    emit_int_insn(adapter, LIBMATTI_ASM_NEWARRAY, arrayType);
}

// Java: public void getStatic(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *owner, const char *name,
                                                     const LIBMATTI_ASM_Type *type)
{
    char *ownerName = internal_name_of(owner);
    emit_field_insn(adapter, LIBMATTI_ASM_GETSTATIC, ownerName, name, LIBMATTI_ASM_Type_GetDescriptor(type));
    free(ownerName);
}

// Java: public void putStatic(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *owner, const char *name,
                                                     const LIBMATTI_ASM_Type *type)
{
    char *ownerName = internal_name_of(owner);
    emit_field_insn(adapter, LIBMATTI_ASM_PUTSTATIC, ownerName, name, LIBMATTI_ASM_Type_GetDescriptor(type));
    free(ownerName);
}

// Java: public void getField(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetField(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *owner, const char *name,
                                                    const LIBMATTI_ASM_Type *type)
{
    char *ownerName = internal_name_of(owner);
    emit_field_insn(adapter, LIBMATTI_ASM_GETFIELD, ownerName, name, LIBMATTI_ASM_Type_GetDescriptor(type));
    free(ownerName);
}

// Java: public void putField(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutField(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *owner, const char *name,
                                                    const LIBMATTI_ASM_Type *type)
{
    char *ownerName = internal_name_of(owner);
    emit_field_insn(adapter, LIBMATTI_ASM_PUTFIELD, ownerName, name, LIBMATTI_ASM_Type_GetDescriptor(type));
    free(ownerName);
}

// Java: public void invokeStatic(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                        const LIBMATTI_ASM_Type *owner,
                                                        const LIBMATTI_ASM_COMMONS_Method *method)
{
    invoke_insn(adapter, LIBMATTI_ASM_INVOKESTATIC, owner, method, 0);
}

// Java: public void invokeVirtual(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeVirtual(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                         const LIBMATTI_ASM_Type *owner,
                                                         const LIBMATTI_ASM_COMMONS_Method *method)
{
    invoke_insn(adapter, LIBMATTI_ASM_INVOKEVIRTUAL, owner, method, 0);
}

// Java: public void invokeConstructor(Type type, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeConstructor(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                             const LIBMATTI_ASM_Type *type,
                                                             const LIBMATTI_ASM_COMMONS_Method *method)
{
    invoke_insn(adapter, LIBMATTI_ASM_INVOKESPECIAL, type, method, 0);
}

// Java: public void invokeInterface(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeInterface(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                           const LIBMATTI_ASM_Type *owner,
                                                           const LIBMATTI_ASM_COMMONS_Method *method)
{
    invoke_insn(adapter, LIBMATTI_ASM_INVOKEINTERFACE, owner, method, 1);
}

// Java: private static Type getBoxedType(Type type)
static const char *boxed_internal_name(const LIBMATTI_ASM_Type *type)
{
    switch (LIBMATTI_ASM_Type_GetSort(type))
    {
        case LIBMATTI_ASM_TYPE_BOOLEAN: return "java/lang/Boolean";
        case LIBMATTI_ASM_TYPE_CHAR: return "java/lang/Character";
        case LIBMATTI_ASM_TYPE_BYTE: return "java/lang/Byte";
        case LIBMATTI_ASM_TYPE_SHORT: return "java/lang/Short";
        case LIBMATTI_ASM_TYPE_INT: return "java/lang/Integer";
        case LIBMATTI_ASM_TYPE_FLOAT: return "java/lang/Float";
        case LIBMATTI_ASM_TYPE_LONG: return "java/lang/Long";
        case LIBMATTI_ASM_TYPE_DOUBLE: return "java/lang/Double";
        default: return NULL;
    }
}

// Java: public void box(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Box(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                               const LIBMATTI_ASM_Type *type)
{
    // Java: only the primitive sorts are boxed; the object sorts are returned unchanged
    const char *boxed = boxed_internal_name(type);
    if (boxed == NULL) return;

    char writer[128];
    snprintf(writer, sizeof(writer), "(%s)V", LIBMATTI_ASM_Type_GetDescriptor(type));

    char owner[64];
    snprintf(owner, sizeof(owner), "L%s;", boxed);

    LIBMATTI_ASM_Type *ownerType = LIBMATTI_ASM_Type_GetType(owner);
    LIBMATTI_ASM_COMMONS_Method *method = LIBMATTI_ASM_COMMONS_Method_New("<init>", writer);
    invoke_insn(adapter, LIBMATTI_ASM_INVOKESPECIAL, ownerType, method, 0);

    LIBMATTI_ASM_COMMONS_Method_Free(method);
    LIBMATTI_ASM_Type_Free(ownerType);
}

// Java: public void valueOf(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ValueOf(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                   const LIBMATTI_ASM_Type *type)
{
    // Java: only the primitive sorts are boxed; the object sorts are returned unchanged
    const char *boxed = boxed_internal_name(type);
    if (boxed == NULL) return;

    char descriptor[128];
    snprintf(descriptor, sizeof(descriptor), "(%s)L%s;", LIBMATTI_ASM_Type_GetDescriptor(type), boxed);

    char owner[64];
    snprintf(owner, sizeof(owner), "L%s;", boxed);

    LIBMATTI_ASM_Type *ownerType = LIBMATTI_ASM_Type_GetType(owner);
    LIBMATTI_ASM_COMMONS_Method *method = LIBMATTI_ASM_COMMONS_Method_New("valueOf", descriptor);
    invoke_insn(adapter, LIBMATTI_ASM_INVOKESTATIC, ownerType, method, 0);

    LIBMATTI_ASM_COMMONS_Method_Free(method);
    LIBMATTI_ASM_Type_Free(ownerType);
}

// Java: public void unbox(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Unbox(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                 const LIBMATTI_ASM_Type *type)
{
    // Java: Type t = NUMBER_TYPE; Method sig = null;
    const char *targetDescriptor = NUMBER_DESCRIPTOR;
    const char *targetName = "java/lang/Number";
    const char *methodName = "intValue";
    const char *methodDescriptor = NUMBER_INT_VALUE_DESCRIPTOR;

    switch (LIBMATTI_ASM_Type_GetSort(type))
    {
        case LIBMATTI_ASM_TYPE_VOID:
            return;
        case LIBMATTI_ASM_TYPE_CHAR:
            targetDescriptor = CHARACTER_DESCRIPTOR;
            targetName = "java/lang/Character";
            methodName = "charValue";
            methodDescriptor = CHAR_CHAR_VALUE_DESCRIPTOR;
            break;
        case LIBMATTI_ASM_TYPE_BOOLEAN:
            targetDescriptor = BOOLEAN_DESCRIPTOR;
            targetName = "java/lang/Boolean";
            methodName = "booleanValue";
            methodDescriptor = BOOLEAN_BOOLEAN_VALUE_DESCRIPTOR;
            break;
        case LIBMATTI_ASM_TYPE_DOUBLE:
            methodName = "doubleValue";
            methodDescriptor = NUMBER_DOUBLE_VALUE_DESCRIPTOR;
            break;
        case LIBMATTI_ASM_TYPE_FLOAT:
            methodName = "floatValue";
            methodDescriptor = NUMBER_FLOAT_VALUE_DESCRIPTOR;
            break;
        case LIBMATTI_ASM_TYPE_LONG:
            methodName = "longValue";
            methodDescriptor = NUMBER_LONG_VALUE_DESCRIPTOR;
            break;
        default:
            break;
    }

    LIBMATTI_ASM_Type *targetType = LIBMATTI_ASM_Type_GetType(targetDescriptor);
    LIBMATTI_ASM_COMMONS_Method *valueMethod = LIBMATTI_ASM_COMMONS_Method_New(methodName, methodDescriptor);

    // Java: checkCast(t); invokeVirtual(t, sig);
    char *internalName = internal_name_of(targetType);
    emit_type_insn(adapter, LIBMATTI_ASM_CHECKCAST, internalName);
    free(internalName);
    invoke_insn(adapter, LIBMATTI_ASM_INVOKEVIRTUAL, targetType, valueMethod, 0);

    (void) targetName;
    LIBMATTI_ASM_COMMONS_Method_Free(valueMethod);
    LIBMATTI_ASM_Type_Free(targetType);
}

// Java: public void cast(Type from, Type to)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Cast(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                const LIBMATTI_ASM_Type *from, const LIBMATTI_ASM_Type *to)
{
    if (LIBMATTI_ASM_Type_Equals(from, to)) return;

    // Java: the primitive conversions (pop/push pairs), then the object casts
    switch (LIBMATTI_ASM_Type_GetSort(from))
    {
        case LIBMATTI_ASM_TYPE_INT:
            switch (LIBMATTI_ASM_Type_GetSort(to))
            {
                case LIBMATTI_ASM_TYPE_BOOLEAN:
                case LIBMATTI_ASM_TYPE_BYTE:
                case LIBMATTI_ASM_TYPE_CHAR:
                case LIBMATTI_ASM_TYPE_SHORT:
                    return;
                case LIBMATTI_ASM_TYPE_LONG:
                    emit_insn(adapter, LIBMATTI_ASM_I2L);
                    return;
                case LIBMATTI_ASM_TYPE_FLOAT:
                    emit_insn(adapter, LIBMATTI_ASM_I2F);
                    return;
                case LIBMATTI_ASM_TYPE_DOUBLE:
                    emit_insn(adapter, LIBMATTI_ASM_I2D);
                    return;
                default:
                    // Java: box(from); if (to.getSort() != OBJECT) cast(to);
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Box(adapter, from);
                    if (LIBMATTI_ASM_Type_GetSort(to) != LIBMATTI_ASM_TYPE_OBJECT)
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Cast(adapter, from, to);
                    return;
            }
        default:
            // Java: if (from.getSort() == OBJECT) { if (to is primitive) unbox(to); else checkCast(to); }
            if (LIBMATTI_ASM_Type_GetSort(from) == LIBMATTI_ASM_TYPE_OBJECT)
            {
                switch (LIBMATTI_ASM_Type_GetSort(to))
                {
                    case LIBMATTI_ASM_TYPE_INT:
                    case LIBMATTI_ASM_TYPE_BOOLEAN:
                    case LIBMATTI_ASM_TYPE_BYTE:
                    case LIBMATTI_ASM_TYPE_CHAR:
                    case LIBMATTI_ASM_TYPE_SHORT:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Unbox(adapter, to);
                        break;
                    default:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_CheckCast(adapter, to);
                        break;
                }
            }
            return;
    }
}

// Java: public void returnValue()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ReturnValue(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    // Java: getReturnType().getOpcode(IRETURN)
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetMethodType(adapter->descriptor);
    LIBMATTI_ASM_Type *returnType = LIBMATTI_ASM_Type_GetReturnType(methodType);
    emit_insn(adapter, LIBMATTI_ASM_Type_GetOpcode(returnType, LIBMATTI_ASM_IRETURN));
    LIBMATTI_ASM_Type_Free(returnType);
    LIBMATTI_ASM_Type_Free(methodType);
}

// Java: public void throwException()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ThrowException(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter)
{
    emit_insn(adapter, LIBMATTI_ASM_ATHROW);
}

// Java: public void throwException(Type type, String msg)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ThrowExceptionWithMessage(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                                    const LIBMATTI_ASM_Type *type,
                                                                    const char *message)
{
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewInstance(adapter, type);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(adapter);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(adapter, message);

    LIBMATTI_ASM_COMMONS_Method *constructor = LIBMATTI_ASM_COMMONS_Method_New(
        LIBMATTI_ASM_COMMONS_METHOD_CONSTRUCTOR_NAME, "(Ljava/lang/String;)V");
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeConstructor(adapter, type, constructor);
    LIBMATTI_ASM_COMMONS_Method_Free(constructor);

    emit_insn(adapter, LIBMATTI_ASM_ATHROW);
}

// Java: private static final Type OBJECT_TYPE = Type.getType(Object.class)
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_OBJECT_TYPE(void)
{
    return LIBMATTI_ASM_Type_GetType(OBJECT_DESCRIPTOR);
}

// Java: private static final Type NUMBER_TYPE = Type.getType(Number.class)
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_NUMBER_TYPE(void)
{
    return LIBMATTI_ASM_Type_GetType(NUMBER_DESCRIPTOR);
}

// Java: private static final Type BOOLEAN_TYPE = Type.getType(Boolean.class)
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_BOOLEAN_TYPE(void)
{
    return LIBMATTI_ASM_Type_GetType(BOOLEAN_DESCRIPTOR);
}

// Java: private static final Type CHARACTER_TYPE = Type.getType(Character.class)
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_CHARACTER_TYPE(void)
{
    return LIBMATTI_ASM_Type_GetType(CHARACTER_DESCRIPTOR);
}
