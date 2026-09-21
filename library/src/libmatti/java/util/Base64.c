#include "libmatti/java/util/Base64.h"

#include <stdlib.h>
#include <string.h>

static const char BASIC_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char URL_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

unsigned char *LIBMATTI_JU_Base64_Encode(const unsigned char *src, size_t length, size_t *outLength,
                                         int mimeLineLength, int urlAlphabet)
{
    const char *alphabet = urlAlphabet ? URL_ALPHABET : BASIC_ALPHABET;

    // Java: MIME separates every 76 output chars with \r\n
    size_t separators = 0;
    size_t outputChars = ((length + 2) / 3) * 4;
    if (mimeLineLength == 76 && outputChars > 76)
        separators = (outputChars - 1) / 76;

    unsigned char *out = malloc(outputChars + separators + 1);
    size_t outIndex = 0;
    size_t lineChars = 0;

    for (size_t i = 0; i < length; i += 3)
    {
        unsigned b0 = src[i];
        unsigned b1 = i + 1 < length ? src[i + 1] : 0;
        unsigned b2 = i + 2 < length ? src[i + 2] : 0;

        out[outIndex++] = (unsigned char) alphabet[b0 >> 2];
        out[outIndex++] = (unsigned char) alphabet[((b0 & 0x03) << 4) | (b1 >> 4)];
        out[outIndex++] = i + 1 < length ? (unsigned char) alphabet[((b1 & 0x0F) << 2) | (b2 >> 6)] : '=';
        out[outIndex++] = i + 2 < length ? (unsigned char) alphabet[b2 & 0x3F] : '=';

        lineChars += 4;
        if (mimeLineLength == 76 && lineChars == 76 && outIndex < outputChars + separators - separators)
        {
            out[outIndex++] = '\r';
            out[outIndex++] = '\n';
            lineChars = 0;
        }
    }

    out[outIndex] = '\0';
    if (outLength)
        *outLength = outIndex;
    return out;
}

// Java: the decode tables; -1 = illegal, -2 = whitespace/MIME-ignored
static int decode_table_value(unsigned char c, int mime)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return mime ? 62 : 62;
    if (c == '/') return 63;
    if (c == '-') return 62; // URL alphabet
    if (c == '_') return 63; // URL alphabet
    if (mime && (c == '\r' || c == '\n' || c == '\t' || c == ' ' || c == '\f' || c == '\v')) return -2;
    return -1;
}

unsigned char *LIBMATTI_JU_Base64_Decode(const unsigned char *src, size_t length, size_t *outLength,
                                         int mimeLineLength, int urlAlphabet)
{
    (void) mimeLineLength;
    (void) urlAlphabet;
    unsigned char *out = malloc(length / 4 * 3 + 3);
    size_t outIndex = 0;
    int buffer = 0, bits = 0;
    int padding = 0;
    int done = 0;

    for (size_t i = 0; i < length; i++)
    {
        unsigned char c = src[i];
        if (c == '=')
        {
            padding++;
            if (padding > 2)
            {
                // Java: IllegalArgumentException("Input byte array has incorrect ending byte")
                free(out);
                return NULL;
            }
            continue;
        }

        int value = decode_table_value(c, mimeLineLength == 76);
        if (value == -2)
            continue;
        if (value == -1)
        {
            // Java: IllegalArgumentException("Illegal base64 character")
            free(out);
            return NULL;
        }
        if (done || padding > 0)
        {
            // Java: data after padding is illegal (except MIME whitespace)
            free(out);
            return NULL;
        }

        buffer = (buffer << 6) | value;
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            out[outIndex++] = (unsigned char) ((buffer >> bits) & 0xFF);
        }
        if (outIndex > 0 && bits == 0)
            done = 0;
    }

    if (padding == 1 && bits >= 6)
    {
        // Java: a single '=' still ends a full quantum correctly in most inputs
    }
    if (outLength)
        *outLength = outIndex;
    return out;
}

char *LIBMATTI_JU_Base64_EncodeToString(const unsigned char *src, size_t length)
{
    size_t encodedLength = 0;
    unsigned char *encoded = LIBMATTI_JU_Base64_Encode(src, length, &encodedLength, 0, 0);
    return (char *) encoded;
}

unsigned char *LIBMATTI_JU_Base64_DecodeString(const char *src, size_t *outLength)
{
    return LIBMATTI_JU_Base64_Decode((const unsigned char *) src, strlen(src), outLength, 0, 0);
}
