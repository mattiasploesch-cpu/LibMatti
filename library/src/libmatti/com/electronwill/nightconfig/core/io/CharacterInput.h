// Port of com.electronwill.nightconfig.core.io.CharacterInput and its ReaderInput implementation.
// Java reads from a Reader and buffers peeked characters in a deque; the port already holds the
// whole document in memory, so the deque collapses into "index + how many characters are peeked".
// Java's readChar()/peekChar() throw ParsingException at the end of the data; C has no exception
// channel, so the port returns -1 (readChar) and reports the failure at the call site.

#ifndef MATTICRAFT_NC_CORE_IO_CHARACTERINPUT_H
#define MATTICRAFT_NC_CORE_IO_CHARACTERINPUT_H

#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"

#include <stddef.h>

// Java: public class ReaderInput extends AbstractInput implements CharacterInput
typedef struct
{
    const char *data;
    size_t length;
    size_t index;
    size_t peekCount;
} LIBMATTI_NC_ReaderInput;

// Java: public ReaderInput(Reader reader)
LIBMATTI_NC_ReaderInput *LIBMATTI_NC_ReaderInput_New(const char *data, size_t length);
void LIBMATTI_NC_ReaderInput_Free(LIBMATTI_NC_ReaderInput *input);

// Java: public int read() / public char readChar() - -1 at the end of the data
int LIBMATTI_NC_ReaderInput_ReadChar(LIBMATTI_NC_ReaderInput *input);
// Java: public int peek()
int LIBMATTI_NC_ReaderInput_Peek(LIBMATTI_NC_ReaderInput *input);
// Java: public int peek(int n)
int LIBMATTI_NC_ReaderInput_PeekN(LIBMATTI_NC_ReaderInput *input, size_t n);
// Java: public char peekChar()
char LIBMATTI_NC_ReaderInput_PeekChar(LIBMATTI_NC_ReaderInput *input);
// Java: public void skipPeeks()
void LIBMATTI_NC_ReaderInput_SkipPeeks(LIBMATTI_NC_ReaderInput *input);
// Java: public void pushBack(char c)
void LIBMATTI_NC_ReaderInput_PushBack(LIBMATTI_NC_ReaderInput *input, char c);
// Java: public CharsWrapper readChars(int n)
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadChars(LIBMATTI_NC_ReaderInput *input, size_t n);
// Java: public CharsWrapper readUntil(char[] stopChars) - the stop character stays unconsumed
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadUntil(LIBMATTI_NC_ReaderInput *input, const char *stopChars,
                                                          size_t stopLength);
// Java: public CharsWrapper readCharsUntil(char[] stopChars) - the stop character stays unconsumed
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_ReaderInput_ReadCharsUntil(LIBMATTI_NC_ReaderInput *input, const char *stopChars,
                                                               size_t stopLength);
// Java: public char readCharAndSkip(char[] skipChars) - -1 at the end of the data
int LIBMATTI_NC_ReaderInput_ReadCharAndSkip(LIBMATTI_NC_ReaderInput *input, const char *skipChars, size_t skipLength);
// Java: public int readAndSkip(char[] skipChars)
int LIBMATTI_NC_ReaderInput_ReadAndSkip(LIBMATTI_NC_ReaderInput *input, const char *skipChars, size_t skipLength);

#endif //MATTICRAFT_NC_CORE_IO_CHARACTERINPUT_H
