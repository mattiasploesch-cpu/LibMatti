// Port of java.text (the surface the loader uses: MessageFormat-style
// formatting and simple date formatting).

#ifndef MATTICRAFT_JAVA_TEXT_TEXT_H
#define MATTICRAFT_JAVA_TEXT_TEXT_H

#include <stddef.h>

// ---------------------------------------------------------------------------
// Java: java.text.MessageFormat - "Pattern {0} {1}" with numbered arguments.
// The port takes the arguments as strings (the callers format their numbers).
// ---------------------------------------------------------------------------

// Java: public static String format(String pattern, Object... arguments)
// Arguments are the NULL-terminated strings in `values`; NULL entries format
// as "null". Caller frees.
char *LIBMATTI_JT_MessageFormat_Format(const char *pattern, const char *const *values, size_t count);

// ---------------------------------------------------------------------------
// Java: java.text.SimpleDateFormat - "yyyy-MM-dd HH:mm:ss"
// The port supports the tokens yyyy, yy, MM, dd, HH, mm, ss and SSS.
// ---------------------------------------------------------------------------

// Java: public static String format(long epochMillis, String pattern) - UTC
char *LIBMATTI_JT_DateFormat_FormatEpoch(long long epochMillis, const char *pattern);

#endif //MATTICRAFT_JAVA_TEXT_TEXT_H
