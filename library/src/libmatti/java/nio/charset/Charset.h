// Port of java.nio.charset (Charset/StandardCharsets/CharBuffer).
// The charsets the loader and game need: UTF-8 (the port's native charset),
// US-ASCII and ISO-8859-1. Java decodes into a CharBuffer; the port decodes
// into a freshly allocated char* (caller frees).

#ifndef MATTICRAFT_JAVA_NIO_CHARSET_H
#define MATTICRAFT_JAVA_NIO_CHARSET_H

#include <stddef.h>
#include <stdint.h>

// Java: StandardCharsets.UTF_8
// Returns a malloc'd UTF-8 string for the given bytes (NUL-terminated for the
// port's convenience); malformed input maps to U+FFFD like Java's REPLACE.
char *LIBMATTI_JN_Charset_DecodeUtf8(const unsigned char *bytes, size_t length);
// Java: StandardCharsets.US_ASCII
char *LIBMATTI_JN_Charset_DecodeAscii(const unsigned char *bytes, size_t length);
// Java: StandardCharsets.ISO_8859_1
char *LIBMATTI_JN_Charset_DecodeIso8859_1(const unsigned char *bytes, size_t length);

// Java: String.getBytes(StandardCharsets.UTF_8) - caller frees, *outLength set
unsigned char *LIBMATTI_JN_Charset_EncodeUtf8(const char *string, size_t *outLength);
// Java: String.getBytes(StandardCharsets.US_ASCII) - non-ASCII becomes '?'
unsigned char *LIBMATTI_JN_Charset_EncodeAscii(const char *string, size_t *outLength);
// Java: String.getBytes(StandardCharsets.ISO_8859_1) - outside 0..255 becomes '?'
unsigned char *LIBMATTI_JN_Charset_EncodeIso8859_1(const char *string, size_t *outLength);

// Java: new String(bytes, charset) picks by name ("UTF-8", "ASCII", "ISO-8859-1")
char *LIBMATTI_JN_Charset_Decode(const char *charsetName, const unsigned char *bytes, size_t length);

// Java: CharBuffer - the decode target; the port keeps the string + position
typedef struct
{
    char *text;      // malloc'd
    size_t length;
    size_t position;
} LIBMATTI_JN_CharBuffer;

LIBMATTI_JN_CharBuffer *LIBMATTI_JN_CharBuffer_Wrap(const char *text);
void LIBMATTI_JN_CharBuffer_Free(LIBMATTI_JN_CharBuffer *buffer);
size_t LIBMATTI_JN_CharBuffer_Remaining(const LIBMATTI_JN_CharBuffer *buffer);

#endif //MATTICRAFT_JAVA_NIO_CHARSET_H
