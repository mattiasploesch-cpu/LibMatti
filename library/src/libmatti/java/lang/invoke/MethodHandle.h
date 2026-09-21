// Port of java.lang.invoke.MethodHandle (the invocation side).
//
// Java's MethodHandle is strongly typed: the handle's MethodType fixes the
// return type, and invokeExact throws when the caller's expectations do not
// match. The C port has no signature checks at the call site, so the contract
// lives in the typedef: the resolved function returns intptr_t, which lets a
// main-method handle (int main(int, char*[])) propagate its exit status and a
// void method simply return 0.

#ifndef MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLE_H
#define MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLE_H

#include "libmatti/java/lang/Class.h"
#include "libmatti/java/lang/invoke/MethodType.h"

#include <stdint.h>

// Java: public abstract class MethodHandle
typedef struct LIBMATTI_JLI_MethodHandle LIBMATTI_JLI_MethodHandle;

// Java: the VM entry point of the resolved method; the intptr_t return lets the
// caller observe an int return (main's exit status) through the void(*) shape
typedef intptr_t (*LIBMATTI_JLI_MethodHandle_Function)(int argc, char *argv[]);

struct LIBMATTI_JLI_MethodHandle
{
    LIBMATTI_JL_Class *declaringClass;
    char *name;
    LIBMATTI_JLI_MethodType *type;
    LIBMATTI_JLI_MethodHandle_Function invokeExact;
};

// Java: public final native @MethodHandle.PolymorphicSignature void invokeExact(Object... args)
// Returns the resolved function's intptr_t result (0 for void methods).
intptr_t LIBMATTI_JLI_MethodHandle_InvokeExact(LIBMATTI_JLI_MethodHandle *handle, int argc, char *argv[]);
void LIBMATTI_JLI_MethodHandle_Free(LIBMATTI_JLI_MethodHandle *handle);

#endif //MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLE_H
