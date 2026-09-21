#include "libmatti/java/lang/AutoCloseable.h"

#include <stddef.h>

// Java: void close() throws Exception
void LIBMATTI_JL_AutoCloseable_Close(LIBMATTI_JL_AutoCloseable *closeable)
{
    if (closeable == NULL) return;

    closeable->close(closeable->self);
}
