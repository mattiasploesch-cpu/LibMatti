// Port of java.lang.AutoCloseable.

#ifndef MATTICRAFT_AUTOCLOSEABLE_H
#define MATTICRAFT_AUTOCLOSEABLE_H

// Java: public interface AutoCloseable
typedef struct LIBMATTI_JL_AutoCloseable LIBMATTI_JL_AutoCloseable;

// Java: void close() throws Exception
typedef void (*LIBMATTI_JL_AutoCloseable_CloseMethod)(void *self);

struct LIBMATTI_JL_AutoCloseable
{
    void *self;
    LIBMATTI_JL_AutoCloseable_CloseMethod close;
};

// Java: void close() throws Exception
void LIBMATTI_JL_AutoCloseable_Close(LIBMATTI_JL_AutoCloseable *closeable);

#endif //MATTICRAFT_AUTOCLOSEABLE_H
