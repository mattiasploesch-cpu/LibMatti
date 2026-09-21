#include "libmatti/java/lang/System.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Java: the standard output/error streams are PrintStreams over the process streams.
typedef struct
{
    LIBMATTI_JI_Writer base;
    FILE *stream;
} SystemStreamWriter;

static void system_stream_write(LIBMATTI_JI_Writer *self, const char *value)
{
    fputs(value, ((SystemStreamWriter *)self)->stream);
}

static void system_stream_flush(LIBMATTI_JI_Writer *self)
{
    fflush(((SystemStreamWriter *)self)->stream);
}

static void system_stream_close(LIBMATTI_JI_Writer *self)
{
    fflush(((SystemStreamWriter *)self)->stream);
}

static LIBMATTI_JI_PrintWriter *make_stream(FILE *stream)
{
    SystemStreamWriter *writer = calloc(1, sizeof(SystemStreamWriter));
    writer->base.write = system_stream_write;
    writer->base.flush = system_stream_flush;
    writer->base.close = system_stream_close;
    writer->stream = stream;
    return LIBMATTI_JI_PrintWriter_New(&writer->base);
}

LIBMATTI_JI_PrintWriter *LIBMATTI_JL_System_Out(void)
{
    static LIBMATTI_JI_PrintWriter *out = NULL;
    if (out == NULL) out = make_stream(stdout);
    return out;
}

LIBMATTI_JI_PrintWriter *LIBMATTI_JL_System_Err(void)
{
    static LIBMATTI_JI_PrintWriter *err = NULL;
    if (err == NULL) err = make_stream(stderr);
    return err;
}

const char *LIBMATTI_JL_System_GetProperty(const char *key)
{
    return getenv(key);
}

const char *LIBMATTI_JL_System_SetProperty(const char *key, const char *value)
{
    setenv(key, value, 1);
    return value;
}

const char *LIBMATTI_JL_System_Getenv(const char *name)
{
    return getenv(name);
}

void LIBMATTI_JL_System_Exit(int status)
{
    exit(status);
}

long long LIBMATTI_JL_System_CurrentTimeMillis(void)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (long long) now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
