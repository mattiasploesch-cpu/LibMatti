// Port of com.electronwill.nightconfig.core.io.ReaderInput.

#include "libmatti/com/electronwill/nightconfig/core/io/CharacterInput.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_NC_ReaderInput *LIBMATTI_NC_ReaderInput_New(const char *data, size_t length)
{
    LIBMATTI_NC_ReaderInput *input = calloc(1, sizeof(*input));
    input->data = data;
    input->length = length;
    input->index = 0;
    input->peekCount = 0;
    return input;
}

void LIBMATTI_NC_ReaderInput_Free(LIBMATTI_NC_ReaderInput *input)
{
    free(input);
}

// Java: AbstractInput.read() consumes a peeked character first
int LIBMATTI_NC_ReaderInput_ReadChar(LIBMATTI_NC_ReaderInput *input)
{
    if (input->index >= input->length) return -1;

    int c = (unsigned char) input->data[input->index++];
    if (input->peekCount > 0) input->peekCount--;
    return c;
}

int LIBMATTI_NC_ReaderInput_Peek(LIBMATTI_NC_ReaderInput *input)
{
    return LIBMATTI_NC_ReaderInput_PeekN(input, 0);
}

int LIBMATTI_NC_ReaderInput_PeekN(LIBMATTI_NC_ReaderInput *input, size_t n)
{
    if (input->index + n >= input->length) return -1;

    if (input->peekCount < n + 1) input->peekCount = n + 1;
    return (unsigned char) input->data[input->index + n];
}

// Java: AbstractInput.peekChar() throws at the end of the data
char LIBMATTI_NC_ReaderInput_PeekChar(LIBMATTI_NC_ReaderInput *input)
{
    return (char) LIBMATTI_NC_ReaderInput_Peek(input);
}

// Java: AbstractInput.skipPeeks() clears the deque, ie consumes the peeked characters
void LIBMATTI_NC_ReaderInput_SkipPeeks(LIBMATTI_NC_ReaderInput *input)
{
    input->index += input->peekCount;
    input->peekCount = 0;
}

// Java: AbstractInput.pushBack(char) adds to the front of the deque
void LIBMATTI_NC_ReaderInput_PushBack(LIBMATTI_NC_ReaderInput *input, char c)
{
    (void) c;
    if (input->index > 0)
    {
        input->index--;
        input->peekCount++;
    }
}

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadChars(LIBMATTI_NC_ReaderInput *input, size_t n)
{
    size_t available = input->length - input->index;
    if (n > available) n = available;

    LIBMATTI_NC_CharsWrapper wrapper = LIBMATTI_NC_CharsWrapper_Of(input->data + input->index, n);
    input->index += n;
    input->peekCount = input->peekCount > n ? input->peekCount - n : 0;
    return wrapper;
}

// Java: AbstractInput.readUntil(char[]) stops before the stop character and pushes it back
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadUntil(LIBMATTI_NC_ReaderInput *input, const char *stopChars,
                                                          size_t stopLength)
{
    size_t start = input->index;
    while (input->index < input->length)
    {
        int found = 0;
        for (size_t i = 0; i < stopLength; i++)
            if (input->data[input->index] == stopChars[i])
            {
                found = 1;
                break;
            }
        if (found) break;

        input->index++;
        if (input->peekCount > 0) input->peekCount--;
    }

    return LIBMATTI_NC_CharsWrapper_Of(input->data + start, input->index - start);
}

LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadCharsUntil(LIBMATTI_NC_ReaderInput *input, const char *stopChars,
                                                               size_t stopLength)
{
    return LIBMATTI_NC_ReaderInput_ReadUntil(input, stopChars, stopLength);
}

// Java: CharacterInput.readCharAndSkip(char[])
int LIBMATTI_NC_ReaderInput_ReadCharAndSkip(LIBMATTI_NC_ReaderInput *input, const char *skipChars, size_t skipLength)
{
    int c;
    do
    {
        c = LIBMATTI_NC_ReaderInput_ReadChar(input);
    }
    while (c != -1 && memchr(skipChars, c, skipLength) != NULL);
    return c;
}

// Java: CharacterInput.readAndSkip(char[])
int LIBMATTI_NC_ReaderInput_ReadAndSkip(LIBMATTI_NC_ReaderInput *input, const char *skipChars, size_t skipLength)
{
    return LIBMATTI_NC_ReaderInput_ReadCharAndSkip(input, skipChars, skipLength);
}
