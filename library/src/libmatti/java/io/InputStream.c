//
// Created by administrator on 09.09.26.
//

#include "InputStream.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JI_InputStream *LIBMATTI_JI_InputStream_Create(const unsigned char *bytes, size_t length)
{
    LIBMATTI_JI_InputStream *stream = calloc(1, sizeof(LIBMATTI_JI_InputStream));

    stream->bytes = malloc(length > 0 ? length : 1);
    memcpy(stream->bytes, bytes, length);
    stream->length = length;
    stream->position = 0;

    return stream;
}

void LIBMATTI_JI_InputStream_Free(LIBMATTI_JI_InputStream *stream)
{
    if (stream == NULL) return;

    free(stream->bytes);
    free(stream);
}

unsigned char *LIBMATTI_JI_InputStream_ReadAllBytes(LIBMATTI_JI_InputStream *stream, size_t *outLength)
{
    size_t remaining = stream->length - stream->position;
    unsigned char *result = malloc(remaining > 0 ? remaining : 1);

    memcpy(result, stream->bytes + stream->position, remaining);
    stream->position = stream->length;

    *outLength = remaining;
    return result;
}