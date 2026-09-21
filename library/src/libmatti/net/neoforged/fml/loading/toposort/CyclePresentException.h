// Port of net.neoforged.fml.loading.toposort.CyclePresentException.

#ifndef MATTICRAFT_FML_LOADING_TOPOSORT_CYCLEPRESENTEXCEPTION_H
#define MATTICRAFT_FML_LOADING_TOPOSORT_CYCLEPRESENTEXCEPTION_H

#include "libmatti/java/lang/Throwable.h"

#include <stddef.h>

// Java: public final class CyclePresentException extends IllegalArgumentException
// Java builds the message from the nodes' toString(); the port takes a name function for that.
typedef struct
{
    LIBMATTI_JL_Throwable base;
    // Java: private final Set<Set<?>> cycles
    void ***cycles;
    size_t *cycleSizes;
    size_t cycleCount;
    const char *(*nodeName)(void *node, void *userdata);
    void *userdata;
} LIBMATTI_FML_CyclePresentException;

// Java: CyclePresentException(Set<Set<?>> cycles) - package private
LIBMATTI_FML_CyclePresentException *LIBMATTI_FML_CyclePresentException_New(
    void ***cycles, size_t *cycleSizes, size_t cycleCount,
    const char *(*nodeName)(void *node, void *userdata), void *userdata);

// Java: public <T> Set<Set<T>> getCycles()
void ***LIBMATTI_FML_CyclePresentException_GetCycles(const LIBMATTI_FML_CyclePresentException *exception,
                                                    size_t **cycleSizes, size_t *cycleCount);

// Java: @Override public String getMessage()
char *LIBMATTI_FML_CyclePresentException_GetMessage(const LIBMATTI_FML_CyclePresentException *exception);

void LIBMATTI_FML_CyclePresentException_Free(LIBMATTI_FML_CyclePresentException *exception);

#endif //MATTICRAFT_FML_LOADING_TOPOSORT_CYCLEPRESENTEXCEPTION_H
