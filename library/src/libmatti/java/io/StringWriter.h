#ifndef MATTICRAFT_JAVA_IO_STRINGWRITER_H
#define MATTICRAFT_JAVA_IO_STRINGWRITER_H

#include "libmatti/java/io/Writer.h"

#include <stddef.h>

// Java: public class StringWriter extends Writer
typedef struct
{
    LIBMATTI_JI_Writer base;
    // Java: private StringBuffer buf
    char *value;
    size_t length;
    size_t capacity;
} LIBMATTI_JI_StringWriter;

// Java: public StringWriter()
LIBMATTI_JI_StringWriter *LIBMATTI_JI_StringWriter_New(void);
void LIBMATTI_JI_StringWriter_Free(LIBMATTI_JI_StringWriter *writer);

// Java: public String toString()
const char *LIBMATTI_JI_StringWriter_ToString(const LIBMATTI_JI_StringWriter *writer);

#endif //MATTICRAFT_JAVA_IO_STRINGWRITER_H
