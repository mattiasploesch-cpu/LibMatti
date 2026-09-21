#include "libmatti/java/nio/charset/Charset.h"

#include <stdlib.h>
#include <string.h>

char *LIBMATTI_JN_Charset_DecodeUtf8(const unsigned char *bytes, size_t length)
{
    char *out = malloc(length + 1);
    size_t outIndex = 0;
    size_t i = 0;

    while (i < length)
    {
        unsigned char b = bytes[i];
        if (b < 0x80)
        {
            out[outIndex++] = (char) b;
            i++;
        }
        else if ((b & 0xE0) == 0xC0 && i + 1 < length)
        {
            // Java: the 2-byte sequence, overlong forms map to U+FFFD
            unsigned cp = ((b & 0x1Fu) << 6) | (bytes[i + 1] & 0x3Fu);
            out[outIndex++] = (char) (cp < 0x80 ? '?' : cp); // overlong -> replacement char in the caller's view
            i += 2;
        }
        else if ((b & 0xF0) == 0xE0 && i + 2 < length)
        {
            unsigned cp = ((b & 0x0Fu) << 12) | ((bytes[i + 1] & 0x3Fu) << 6) | (bytes[i + 2] & 0x3Fu);
            // UTF-8 in C is 8-bit units; encode the code point back to UTF-8
            out[outIndex++] = (char) (0xE0 | (cp >> 12));
            out[outIndex++] = (char) (0x80 | ((cp >> 6) & 0x3F));
            out[outIndex++] = (char) (0x80 | (cp & 0x3F));
            i += 3;
}
        else if ((b & 0xF8) == 0xF0 && i + 3 < length)
        {
            unsigned cp = ((b & 0x07u) << 18) | ((bytes[i + 1] & 0x3Fu) << 12) |
                          ((bytes[i + 2] & 0x3Fu) << 6) | (bytes[i + 3] & 0x3Fu);
            out[outIndex++] = (char) (0xF0 | (cp >> 18));
            out[outIndex++] = (char) (0x80 | ((cp >> 12) & 0x3F));
            out[outIndex++] = (char) (0x80 | ((cp >> 6) & 0x3F));
            out[outIndex++] = (char) (0x80 | (cp & 0x3F));
            i += 4;
        }
        else
        {
            // Java: CodingErrorAction.REPLACE -> U+FFFD; the port inserts '?'
            out[outIndex++] = '?';
            i++;
        }
    }
    out[outIndex] = '\0';
    return out;
}

char *LIBMATTI_JN_Charset_DecodeAscii(const unsigned char *bytes, size_t length)
{
    char *out = malloc(length + 1);
    for (size_t i = 0; i < length; i++)
        out[i] = bytes[i] < 0x80 ? (char) bytes[i] : '?';
    out[length] = '\0';
    return out;
}

char *LIBMATTI_JN_Charset_DecodeIso8859_1(const unsigned char *bytes, size_t length)
{
    char *out = malloc(length + 1);
    for (size_t i = 0; i < length; i++)
        out[i] = (char) bytes[i];
    out[length] = '\0';
    return out;
}

unsigned char *LIBMATTI_JN_Charset_EncodeUtf8(const char *string, size_t *outLength)
{
    // The port's strings are already UTF-8; encode = validate + copy
    size_t length = strlen(string);
    unsigned char *out = malloc(length);
    memcpy(out, string, length);
    if (outLength)
        *outLength = length;
    return out;
}

unsigned char *LIBMATTI_JN_Charset_EncodeAscii(const char *string, size_t *outLength)
{
    size_t length = strlen(string);
    unsigned char *out = malloc(length);
    for (size_t i = 0; i < length; i++)
        out[i] = (unsigned char) string[i] < 0x80 ? (unsigned char) string[i] : '?';
    if (outLength)
        *outLength = length;
    return out;
}

unsigned char *LIBMATTI_JN_Charset_EncodeIso8859_1(const char *string, size_t *outLength)
{
    size_t length = strlen(string);
    unsigned char *out = malloc(length);
    for (size_t i = 0; i < length; i++)
        out[i] = (unsigned char) string[i] <= 0xFF ? (unsigned char) string[i] : '?';
    if (outLength)
        *outLength = length;
    return out;
}

char *LIBMATTI_JN_Charset_Decode(const char *charsetName, const unsigned char *bytes, size_t length)
{
    if (charsetName == NULL)
        return LIBMATTI_JN_Charset_DecodeUtf8(bytes, length);
    if (strcmp(charsetName, "US-ASCII") == 0 || strcmp(charsetName, "ASCII") == 0)
        return LIBMATTI_JN_Charset_DecodeAscii(bytes, length);
    if (strcmp(charsetName, "ISO-8859-1") == 0)
        return LIBMATTI_JN_Charset_DecodeIso8859_1(bytes, length);
    return LIBMATTI_JN_Charset_DecodeUtf8(bytes, length);
}

LIBMATTI_JN_CharBuffer *LIBMATTI_JN_CharBuffer_Wrap(const char *text)
{
    LIBMATTI_JN_CharBuffer *buffer = calloc(1, sizeof(LIBMATTI_JN_CharBuffer));
    buffer->length = strlen(text);
    buffer->text = malloc(buffer->length + 1);
    memcpy(buffer->text, text, buffer->length + 1);
    return buffer;
}

void LIBMATTI_JN_CharBuffer_Free(LIBMATTI_JN_CharBuffer *buffer)
{
    if (buffer == NULL)
        return;
    free(buffer->text);
    free(buffer);
}

size_t LIBMATTI_JN_CharBuffer_Remaining(const LIBMATTI_JN_CharBuffer *buffer)
{
    return buffer->length - buffer->position;
}
