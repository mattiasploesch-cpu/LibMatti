









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

#endif
