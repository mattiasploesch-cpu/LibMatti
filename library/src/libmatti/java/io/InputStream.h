//
// Created by administrator on 09.09.26.
//
// Port of java.io.InputStream.

#ifndef MATTICRAFT_INPUTSTREAM_H
#define MATTICRAFT_INPUTSTREAM_H

#include <stddef.h>

// Minimal byte-buffer stream (Java: java.io.InputStream)
typedef struct
{
    unsigned char *bytes;
    size_t length;
    size_t position;
} LIBMATTI_JI_InputStream;

// Copies the given bytes into a new stream (Java: ByteArrayInputStream)
LIBMATTI_JI_InputStream *LIBMATTI_JI_InputStream_Create(const unsigned char *bytes, size_t length);
void LIBMATTI_JI_InputStream_Free(LIBMATTI_JI_InputStream *stream);

// Java: readAllBytes() - returns the remaining bytes from the current position
// as a newly allocated buffer (caller frees), advancing the position
unsigned char *LIBMATTI_JI_InputStream_ReadAllBytes(LIBMATTI_JI_InputStream *stream, size_t *outLength);

#endif //MATTICRAFT_INPUTSTREAM_H