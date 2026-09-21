// Port of com.electronwill.nightconfig.core.io.Utils.

#ifndef MATTICRAFT_NC_CORE_IO_UTILS_H
#define MATTICRAFT_NC_CORE_IO_UTILS_H

#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"

// Java: public static int parseInt(CharSequence csq, int radix) throws NumberFormatException
int LIBMATTI_NC_Utils_ParseInt(const LIBMATTI_NC_CharsWrapper *chars, int radix, int *ok);
// Java: public static long parseLong(CharSequence csq, int radix) throws NumberFormatException
long long LIBMATTI_NC_Utils_ParseLong(const LIBMATTI_NC_CharsWrapper *chars, int radix, int *ok);
// Java: public static double parseDouble(CharSequence csq) throws NumberFormatException
double LIBMATTI_NC_Utils_ParseDouble(const LIBMATTI_NC_CharsWrapper *chars, int *ok);

// the UTF-8 encoding of a code point (Java: new String(new int[] { codePoint }, 0, 1))
size_t LIBMATTI_NC_Utils_EncodeUtf8(int codePoint, char out[4]);

#endif //MATTICRAFT_NC_CORE_IO_UTILS_H
