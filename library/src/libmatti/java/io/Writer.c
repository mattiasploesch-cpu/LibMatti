#include "libmatti/java/io/Writer.h"

void LIBMATTI_JI_Writer_Write(LIBMATTI_JI_Writer *writer, const char *value)
{
    writer->write(writer, value);
}

void LIBMATTI_JI_Writer_Flush(LIBMATTI_JI_Writer *writer)
{
    writer->flush(writer);
}

void LIBMATTI_JI_Writer_Close(LIBMATTI_JI_Writer *writer)
{
    writer->close(writer);
}
