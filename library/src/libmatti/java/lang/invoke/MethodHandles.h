#ifndef MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLES_H
#define MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLES_H

#include "libmatti/java/lang/Class.h"
#include "libmatti/java/lang/invoke/MethodHandle.h"
#include "libmatti/java/lang/invoke/MethodType.h"

// Java: public class MethodHandles.Lookup
typedef struct
{
    int unused;
} LIBMATTI_JLI_MethodHandles_Lookup;

// Java: public static MethodHandles.Lookup publicLookup()
LIBMATTI_JLI_MethodHandles_Lookup *LIBMATTI_JLI_MethodHandles_PublicLookup(void);

// Java: public MethodHandle findStatic(Class<?> refc, String name, MethodType type)
//       throws NoSuchMethodException - NULL in the C port when the method is not registered
LIBMATTI_JLI_MethodHandle *LIBMATTI_JLI_MethodHandles_Lookup_FindStatic(LIBMATTI_JLI_MethodHandles_Lookup *lookup,
                                                                       LIBMATTI_JL_Class *refc, const char *name,
                                                                       LIBMATTI_JLI_MethodType *type);

#endif //MATTICRAFT_JAVA_LANG_INVOKE_METHODHANDLES_H
