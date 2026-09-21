#ifndef MATTICRAFT_JAVA_IO_PRINTWRITER_H
#define MATTICRAFT_JAVA_IO_PRINTWRITER_H

#include "libmatti/java/io/Writer.h"

// Port of java.io.PrintWriter.
typedef struct
{
    // Java: the underlying Writer
    LIBMATTI_JI_Writer *out;
} LIBMATTI_JI_PrintWriter;

// Java: public PrintWriter(Writer out)
LIBMATTI_JI_PrintWriter *LIBMATTI_JI_PrintWriter_New(LIBMATTI_JI_Writer *out);
void LIBMATTI_JI_PrintWriter_Free(LIBMATTI_JI_PrintWriter *writer);

// Java: public void print(String s)
void LIBMATTI_JI_PrintWriter_Print(LIBMATTI_JI_PrintWriter *writer, const char *value);
// Java: public void println(String x)
void LIBMATTI_JI_PrintWriter_Println(LIBMATTI_JI_PrintWriter *writer, const char *value);
// Java: public void flush()
void LIBMATTI_JI_PrintWriter_Flush(LIBMATTI_JI_PrintWriter *writer);

#endif //MATTICRAFT_JAVA_IO_PRINTWRITER_H
