









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
