#ifndef MATTICRAFT_JAVA_LANG_REFLECT_INVOCATIONTARGETEXCEPTION_H
#define MATTICRAFT_JAVA_LANG_REFLECT_INVOCATIONTARGETEXCEPTION_H

#include "libmatti/java/lang/Throwable.h"

// Java: public class InvocationTargetException extends ReflectiveOperationException
typedef struct
{
    LIBMATTI_JL_Throwable base;
} LIBMATTI_JLR_InvocationTargetException;

#define LIBMATTI_JLR_InvocationTargetException_NAME "java.lang.reflect.InvocationTargetException"

// Java: public InvocationTargetException(Throwable target)
LIBMATTI_JLR_InvocationTargetException *LIBMATTI_JLR_InvocationTargetException_New(LIBMATTI_JL_Throwable *target);
void LIBMATTI_JLR_InvocationTargetException_Free(LIBMATTI_JLR_InvocationTargetException *exception);

// Java: public Throwable getTargetException()
LIBMATTI_JL_Throwable *LIBMATTI_JLR_InvocationTargetException_GetTargetException(
    const LIBMATTI_JLR_InvocationTargetException *exception);

#endif //MATTICRAFT_JAVA_LANG_REFLECT_INVOCATIONTARGETEXCEPTION_H
