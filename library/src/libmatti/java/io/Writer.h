#ifndef MATTICRAFT_JAVA_IO_WRITER_H
#define MATTICRAFT_JAVA_IO_WRITER_H

// Port of java.io.Writer.
typedef struct LIBMATTI_JI_Writer LIBMATTI_JI_Writer;

struct LIBMATTI_JI_Writer
{
    // Java: public void write(String str)
    void (*write)(LIBMATTI_JI_Writer *self, const char *value);
    // Java: public abstract void flush()
    void (*flush)(LIBMATTI_JI_Writer *self);
    // Java: public abstract void close()
    void (*close)(LIBMATTI_JI_Writer *self);
};

// Java: public void write(String str)
void LIBMATTI_JI_Writer_Write(LIBMATTI_JI_Writer *writer, const char *value);
// Java: public abstract void flush()
void LIBMATTI_JI_Writer_Flush(LIBMATTI_JI_Writer *writer);
// Java: public abstract void close()
void LIBMATTI_JI_Writer_Close(LIBMATTI_JI_Writer *writer);

#endif //MATTICRAFT_JAVA_IO_WRITER_H
