#include "libmatti/net/neoforged/fml/loading/toposort/CyclePresentException.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: CyclePresentException(Set<Set<?>> cycles) { super("Cycles present in graph: " + cycles); this.cycles = cycles; }
LIBMATTI_FML_CyclePresentException *LIBMATTI_FML_CyclePresentException_New(
    void ***cycles, size_t *cycleSizes, size_t cycleCount,
    const char *(*nodeName)(void *node, void *userdata), void *userdata)
{
    LIBMATTI_FML_CyclePresentException *exception = calloc(1, sizeof(LIBMATTI_FML_CyclePresentException));
    exception->cycles = cycles;
    exception->cycleSizes = cycleSizes;
    exception->cycleCount = cycleCount;
    exception->nodeName = nodeName;
    exception->userdata = userdata;
    return exception;
}

// Java: public <T> Set<Set<T>> getCycles()
void ***LIBMATTI_FML_CyclePresentException_GetCycles(const LIBMATTI_FML_CyclePresentException *exception,
                                                    size_t **cycleSizes, size_t *cycleCount)
{
    *cycleSizes = exception->cycleSizes;
    *cycleCount = exception->cycleCount;
    return exception->cycles;
}

// Java: @Override public String getMessage() { return "Cycles present in graph: " + cycles; }
char *LIBMATTI_FML_CyclePresentException_GetMessage(const LIBMATTI_FML_CyclePresentException *exception)
{
    char *message = strdup("Cycles present in graph: [");
    size_t length = strlen(message);

    for (size_t i = 0; i < exception->cycleCount; i++)
    {
        if (i > 0)
        {
            message = realloc(message, length + 2);
            message[length++] = ',';
            message[length] = '\0';
        }

        message = realloc(message, length + 2);
        message[length++] = '[';
        message[length] = '\0';

        for (size_t j = 0; j < exception->cycleSizes[i]; j++)
        {
            const char *name = exception->nodeName != NULL
                                   ? exception->nodeName(exception->cycles[i][j], exception->userdata)
                                   : "";
            size_t nameLength = strlen(name);
            message = realloc(message, length + nameLength + 2);
            memcpy(message + length, name, nameLength);
            length += nameLength;
            message[length] = '\0';

            if (j + 1 < exception->cycleSizes[i])
            {
                message = realloc(message, length + 2);
                message[length++] = ',';
                message[length] = '\0';
            }
        }

        message = realloc(message, length + 2);
        message[length++] = ']';
        message[length] = '\0';
    }

    message = realloc(message, length + 2);
    message[length++] = ']';
    message[length] = '\0';
    return message;
}

void LIBMATTI_FML_CyclePresentException_Free(LIBMATTI_FML_CyclePresentException *exception)
{
    if (exception == NULL) return;
    for (size_t i = 0; i < exception->cycleCount; i++) free(exception->cycles[i]);
    free(exception->cycles);
    free(exception->cycleSizes);
    free(exception);
}
