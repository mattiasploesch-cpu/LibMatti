// Port of org.objectweb.asm.commons.GeneratorAdapter.
//
// Java: GeneratorAdapter extends LocalVariablesSorter extends MethodVisitor, i.e. it is the
// convenience front end of a MethodVisitor chain. The C port has no MethodVisitor: the ASM tree
// node classes are written directly, so the adapter appends its instructions to an InsnList
// (exactly what the Java ListGeneratorAdapter does by assigning mv = method).
// Because LocalVariablesSorter's whole purpose is to remap MethodVisitor local variable slots and
// to bridge visitMaxs/visitFrame, it has no counterpart here; the newLocal counter is kept so the
// adapter's local-variable contract stays the same.

#ifndef MATTICRAFT_ASM_COMMONS_GENERATORADAPTER_H
#define MATTICRAFT_ASM_COMMONS_GENERATORADAPTER_H

#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/commons/Method.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"

#include <stddef.h>

// Java: the arithmetic opcode constants (aliases of Opcodes)
#define LIBMATTI_ASM_COMMONS_ADD LIBMATTI_ASM_IADD
#define LIBMATTI_ASM_COMMONS_SUB LIBMATTI_ASM_ISUB
#define LIBMATTI_ASM_COMMONS_MUL LIBMATTI_ASM_IMUL
#define LIBMATTI_ASM_COMMONS_DIV LIBMATTI_ASM_IDIV
#define LIBMATTI_ASM_COMMONS_REM LIBMATTI_ASM_IREM
#define LIBMATTI_ASM_COMMONS_NEG LIBMATTI_ASM_INEG
#define LIBMATTI_ASM_COMMONS_SHL LIBMATTI_ASM_ISHL
#define LIBMATTI_ASM_COMMONS_SHR LIBMATTI_ASM_ISHR
#define LIBMATTI_ASM_COMMONS_USHR LIBMATTI_ASM_IUSHR
#define LIBMATTI_ASM_COMMONS_AND LIBMATTI_ASM_IAND
#define LIBMATTI_ASM_COMMONS_OR LIBMATTI_ASM_IOR
#define LIBMATTI_ASM_COMMONS_XOR LIBMATTI_ASM_IXOR

// Java: public class GeneratorAdapter extends LocalVariablesSorter
typedef struct LIBMATTI_ASM_COMMONS_GeneratorAdapter
{
    // Java: protected MethodVisitor mv - the port's MethodVisitor counterpart is the tree list
    LIBMATTI_ASMT_InsnList *insnList;

    // Java: private final int access / private final String name / private final String descriptor
    int access;
    char *name;
    char *descriptor;

    // Java: LocalVariablesSorter.nextLocal
    int nextLocal;
    // Java: LocalVariablesSorter.firstLocal
    int firstLocal;
} LIBMATTI_ASM_COMMONS_GeneratorAdapter;

// Java: protected GeneratorAdapter(int api, MethodVisitor methodVisitor, int access, String name, String descriptor)
LIBMATTI_ASM_COMMONS_GeneratorAdapter *LIBMATTI_ASM_COMMONS_GeneratorAdapter_New(
    int api, int access, const char *name, const char *descriptor, LIBMATTI_ASMT_InsnList *insnList);
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Free(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);

// Java: LocalVariablesSorter.newLocal(Type type)
int LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewLocal(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                   const LIBMATTI_ASM_Type *type);

// Java: public void push(int value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int value);
// Java: public void push(boolean value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushBool(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int value);
// Java: public void push(long value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushLong(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, long long value);
// Java: public void push(float value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushFloat(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, float value);
// Java: public void push(double value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushDouble(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, double value);
// Java: public void push(String value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const char *value);
// Java: public void push(Type value)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushType(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *value);

// Java: public void dup()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);
// Java: public void swap()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Swap(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);

// Java: public void arrayLength()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayLength(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);
// Java: public void arrayStore(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayStore(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const LIBMATTI_ASM_Type *type);

// Java: public void math(int op, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Math(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter, int op,
                                                const LIBMATTI_ASM_Type *type);
// Java: public void not()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Not(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);

// Java: public void checkCast(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_CheckCast(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *type);
// Java: public void instanceOf(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InstanceOf(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                      const LIBMATTI_ASM_Type *type);
// Java: public void newInstance(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewInstance(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                       const LIBMATTI_ASM_Type *type);
// Java: public void newArray(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewArray(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *type);

// Java: public void getStatic(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *owner, const char *name,
                                                     const LIBMATTI_ASM_Type *type);
// Java: public void putStatic(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                     const LIBMATTI_ASM_Type *owner, const char *name,
                                                     const LIBMATTI_ASM_Type *type);
// Java: public void getField(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetField(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *owner, const char *name,
                                                    const LIBMATTI_ASM_Type *type);
// Java: public void putField(Type owner, String name, Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutField(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                    const LIBMATTI_ASM_Type *owner, const char *name,
                                                    const LIBMATTI_ASM_Type *type);

// Java: public void invokeStatic(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                        const LIBMATTI_ASM_Type *owner,
                                                        const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public void invokeVirtual(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeVirtual(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                         const LIBMATTI_ASM_Type *owner,
                                                         const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public void invokeConstructor(Type type, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeConstructor(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                             const LIBMATTI_ASM_Type *type,
                                                             const LIBMATTI_ASM_COMMONS_Method *method);
// Java: public void invokeInterface(Type owner, Method method)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeInterface(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                           const LIBMATTI_ASM_Type *owner,
                                                           const LIBMATTI_ASM_COMMONS_Method *method);

// Java: public void box(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Box(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                               const LIBMATTI_ASM_Type *type);
// Java: public void valueOf(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ValueOf(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                   const LIBMATTI_ASM_Type *type);
// Java: public void unbox(Type type)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Unbox(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                 const LIBMATTI_ASM_Type *type);
// Java: public void cast(Type from, Type to)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_Cast(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                const LIBMATTI_ASM_Type *from, const LIBMATTI_ASM_Type *to);

// Java: public void returnValue()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ReturnValue(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);
// Java: public void throwException()
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ThrowException(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter);
// Java: public void throwException(Type type, String msg)
void LIBMATTI_ASM_COMMONS_GeneratorAdapter_ThrowExceptionWithMessage(LIBMATTI_ASM_COMMONS_GeneratorAdapter *adapter,
                                                                    const LIBMATTI_ASM_Type *type,
                                                                    const char *message);

// Java: the static Type constants GeneratorAdapter uses for boxing
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_OBJECT_TYPE(void);
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_NUMBER_TYPE(void);
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_BOOLEAN_TYPE(void);
LIBMATTI_ASM_Type *LIBMATTI_ASM_COMMONS_GeneratorAdapter_CHARACTER_TYPE(void);

#endif //MATTICRAFT_ASM_COMMONS_GENERATORADAPTER_H
