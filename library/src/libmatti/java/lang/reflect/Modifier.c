// Port of java.lang.reflect.Modifier.

#include "libmatti/java/lang/reflect/Modifier.h"

// Java: public static boolean isPublic(int mod)
int LIBMATTI_JL_Modifier_IsPublic(int mod)
{
    return (mod & LIBMATTI_JL_Modifier_PUBLIC) != 0;
}

// Java: public static boolean isStatic(int mod)
int LIBMATTI_JL_Modifier_IsStatic(int mod)
{
    return (mod & LIBMATTI_JL_Modifier_STATIC) != 0;
}

// Java: public static boolean isFinal(int mod)
int LIBMATTI_JL_Modifier_IsFinal(int mod)
{
    return (mod & LIBMATTI_JL_Modifier_FINAL) != 0;
}

// Java: public static boolean isAbstract(int mod)
int LIBMATTI_JL_Modifier_IsAbstract(int mod)
{
    return (mod & LIBMATTI_JL_Modifier_ABSTRACT) != 0;
}

// Java: public static boolean isInterface(int mod)
int LIBMATTI_JL_Modifier_IsInterface(int mod)
{
    return (mod & LIBMATTI_JL_Modifier_INTERFACE) != 0;
}
