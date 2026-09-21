// Port of com.electronwill.nightconfig.core.io.Utils.

#include "libmatti/com/electronwill/nightconfig/core/io/Utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

static char *toCString(const LIBMATTI_NC_CharsWrapper *chars)
{
    char *copy = malloc(chars->length + 1);
    for (size_t i = 0; i < chars->length; i++) copy[i] = chars->data[i];
    copy[chars->length] = '\0';
    return copy;
}

long long LIBMATTI_NC_Utils_ParseLong(const LIBMATTI_NC_CharsWrapper *chars, int radix, int *ok)
{
    *ok = 0;
    if (chars->length == 0) return 0;

    char *copy = toCString(chars);
    char *end = NULL;
    errno = 0;
    long long value = strtoll(copy, &end, radix);
    int valid = end == copy + chars->length && errno == 0;
    free(copy);

    if (!valid) return 0;
    *ok = 1;
    return value;
}

int LIBMATTI_NC_Utils_ParseInt(const LIBMATTI_NC_CharsWrapper *chars, int radix, int *ok)
{
    int valid = 0;
    long long value = LIBMATTI_NC_Utils_ParseLong(chars, radix, &valid);
    *ok = valid;
    return valid ? (int) value : 0;
}

double LIBMATTI_NC_Utils_ParseDouble(const LIBMATTI_NC_CharsWrapper *chars, int *ok)
{
    *ok = 0;
    if (chars->length == 0) return 0.0;

    char *copy = toCString(chars);
    char *end = NULL;
    errno = 0;
    double value = strtod(copy, &end);
    int valid = end == copy + chars->length && errno == 0;
    free(copy);

    if (!valid) return 0.0;
    *ok = 1;
    return value;
}

size_t LIBMATTI_NC_Utils_EncodeUtf8(int codePoint, char out[4])
{
    if (codePoint < 0x80)
    {
        out[0] = (char) codePoint;
        return 1;
    }
    if (codePoint < 0x800)
    {
        out[0] = (char) (0xC0 | (codePoint >> 6));
        out[1] = (char) (0x80 | (codePoint & 0x3F));
        return 2;
    }
    if (codePoint < 0x10000)
    {
        out[0] = (char) (0xE0 | (codePoint >> 12));
        out[1] = (char) (0x80 | ((codePoint >> 6) & 0x3F));
        out[2] = (char) (0x80 | (codePoint & 0x3F));
        return 3;
    }

    out[0] = (char) (0xF0 | (codePoint >> 18));
    out[1] = (char) (0x80 | ((codePoint >> 12) & 0x3F));
    out[2] = (char) (0x80 | ((codePoint >> 6) & 0x3F));
    out[3] = (char) (0x80 | (codePoint & 0x3F));
    return 4;
}
