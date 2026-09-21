#include "libmatti/java/io/PrintWriter.h"

#include <stdlib.h>

LIBMATTI_JI_PrintWriter *LIBMATTI_JI_PrintWriter_New(LIBMATTI_JI_Writer *out)
{
    LIBMATTI_JI_PrintWriter *writer = calloc(1, sizeof(LIBMATTI_JI_PrintWriter));
    writer->out = out;
    return writer;
}

void LIBMATTI_JI_PrintWriter_Free(LIBMATTI_JI_PrintWriter *writer)
{
    free(writer);
}

void LIBMATTI_JI_PrintWriter_Print(LIBMATTI_JI_PrintWriter *writer, const char *value)
{
    LIBMATTI_JI_Writer_Write(writer->out, value);
}

// Java: public void println(String x) { print(x); newLine(); } - the platform line separator
void LIBMATTI_JI_PrintWriter_Println(LIBMATTI_JI_PrintWriter *writer, const char *value)
{
    LIBMATTI_JI_Writer_Write(writer->out, value);
    LIBMATTI_JI_Writer_Write(writer->out, "\n");
}

void LIBMATTI_JI_PrintWriter_Flush(LIBMATTI_JI_PrintWriter *writer)
{
    LIBMATTI_JI_Writer_Flush(writer->out);
}
