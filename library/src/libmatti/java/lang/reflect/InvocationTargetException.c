#include "libmatti/java/lang/reflect/InvocationTargetException.h"

#include <stdlib.h>
#include <string.h>

// Java: public InvocationTargetException(Throwable target) { super((Throwable) null); this.target = target; }
LIBMATTI_JLR_InvocationTargetException *LIBMATTI_JLR_InvocationTargetException_New(LIBMATTI_JL_Throwable *target)
{
    LIBMATTI_JLR_InvocationTargetException *exception = calloc(1, sizeof(LIBMATTI_JLR_InvocationTargetException));
    exception->base.className = strdup(LIBMATTI_JLR_InvocationTargetException_NAME);

    // Java: public Throwable getCause() { return target; }
    exception->base.cause = target;
    return exception;
}

void LIBMATTI_JLR_InvocationTargetException_Free(LIBMATTI_JLR_InvocationTargetException *exception)
{
    LIBMATTI_JL_Throwable_Free(&exception->base);
}

// Java: public Throwable getTargetException() { return target; }
LIBMATTI_JL_Throwable *LIBMATTI_JLR_InvocationTargetException_GetTargetException(
    const LIBMATTI_JLR_InvocationTargetException *exception)
{
    return exception->base.cause;
}
