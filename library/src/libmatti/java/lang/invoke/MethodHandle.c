// Port of java.lang.invoke.MethodHandle (the invocation side).
//
// Java's MethodHandle is strongly typed: the handle's MethodType fixes the
// return type, and invokeExact throws when the caller's expectations do not
// match. The C port has no signature checks at the call site, so the contract
// lives in the typedef: the resolved function returns intptr_t, which lets a
// main-method handle (int main(int, char*[])) propagate its exit status and a
// void method simply return 0.

#include "libmatti/java/lang/invoke/MethodHandle.h"

#include <stdlib.h>
#include <string.h>

// Java: public final native @MethodHandle.PolymorphicSignature void invokeExact(Object... args)
// Returns the resolved function's intptr_t result (0 for void methods).
intptr_t LIBMATTI_JLI_MethodHandle_InvokeExact(LIBMATTI_JLI_MethodHandle *handle, int argc, char *argv[])
{
    return handle->invokeExact(argc, argv);
}

void LIBMATTI_JLI_MethodHandle_Free(LIBMATTI_JLI_MethodHandle *handle)
{
    if (handle == NULL) return;
    free(handle->name);
    LIBMATTI_JLI_MethodType_Free(handle->type);
    free(handle);
}
