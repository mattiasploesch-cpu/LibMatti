#include "libmatti/java/io/StringWriter.h"

#include <stdlib.h>
#include <string.h>

static void string_writer_write(LIBMATTI_JI_Writer *self, const char *value)
{
    LIBMATTI_JI_StringWriter *writer = (LIBMATTI_JI_StringWriter *)self;
    size_t length = strlen(value);
    size_t needed = writer->length + length + 1;

    if (needed > writer->capacity)
    {
        writer->capacity = needed * 2;
        writer->value = realloc(writer->value, writer->capacity);
    }

    memcpy(writer->value + writer->length, value, length);
    writer->length += length;
    writer->value[writer->length] = '\0';
}

// Java: public void flush() {} - the StringWriter buffer is not flushed
static void string_writer_flush(LIBMATTI_JI_Writer *self)
{
    (void)self;
}

// Java: public void close() {} - closing a StringWriter has no effect
static void string_writer_close(LIBMATTI_JI_Writer *self)
{
    (void)self;
}

LIBMATTI_JI_StringWriter *LIBMATTI_JI_StringWriter_New(void)
{
    LIBMATTI_JI_StringWriter *writer = calloc(1, sizeof(LIBMATTI_JI_StringWriter));
    writer->base.write = string_writer_write;
    writer->base.flush = string_writer_flush;
    writer->base.close = string_writer_close;

    writer->capacity = 32;
    writer->value = calloc(1, writer->capacity);
    return writer;
}

void LIBMATTI_JI_StringWriter_Free(LIBMATTI_JI_StringWriter *writer)
{
    if (writer == NULL) return;
    free(writer->value);
    free(writer);
}

const char *LIBMATTI_JI_StringWriter_ToString(const LIBMATTI_JI_StringWriter *writer)
{
    return writer->value;
}
