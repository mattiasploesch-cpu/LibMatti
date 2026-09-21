#include "libmatti/java/lang/invoke/MethodHandles.h"

#include <stdlib.h>
#include <string.h>

// Java: public static MethodHandles.Lookup publicLookup()
LIBMATTI_JLI_MethodHandles_Lookup *LIBMATTI_JLI_MethodHandles_PublicLookup(void)
{
    static LIBMATTI_JLI_MethodHandles_Lookup lookup;
    return &lookup;
}

// Java: public MethodHandle findStatic(Class<?> refc, String name, MethodType type)
LIBMATTI_JLI_MethodHandle *LIBMATTI_JLI_MethodHandles_Lookup_FindStatic(LIBMATTI_JLI_MethodHandles_Lookup *lookup,
                                                                       LIBMATTI_JL_Class *refc, const char *name,
                                                                       LIBMATTI_JLI_MethodType *type)
{
    (void)lookup;

    // Java: the VM resolves the static method on the class
    LIBMATTI_JL_Class_StaticMethod *method = LIBMATTI_JL_Class_FindStaticMethod(refc, name);
    if (method == NULL) return NULL;

    LIBMATTI_JLI_MethodHandle *handle = calloc(1, sizeof(LIBMATTI_JLI_MethodHandle));
    handle->declaringClass = refc;
    handle->name = strdup(name);
    handle->type = type;
    handle->invokeExact = (LIBMATTI_JLI_MethodHandle_Function) method->function;
    return handle;
}
