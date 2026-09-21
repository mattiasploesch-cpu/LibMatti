// Port of org.apache.commons.lang3.text.StrSubstitutor and
// org.apache.commons.lang3.text.ExtendedMessageFormat (the surface log4j's
// pattern layout uses through the TODO in the log4j port).

#ifndef MATTICRAFT_COMMONS_LANG3_TEXT_H
#define MATTICRAFT_COMMONS_LANG3_TEXT_H

#include <stddef.h>

// Java: StrSubstitutor.replace(String) with the default "${var}" prefix/suffix.
// lookup: the callback resolves a variable name; NULL = leave the variable as is.
typedef char *(*LIBMATTI_CL3_StrLookup)(const char *variable, void *self);

// Java: public String replace(final String source) - caller frees
char *LIBMATTI_CL3_StrSubstitutor_Replace(const char *source, LIBMATTI_CL3_StrLookup lookup, void *self);

// Java: ExtendedMessageFormat.format(format, arguments) - '{0}'-style plus
// the choice/date extensions are folded into the MessageFormat port
char *LIBMATTI_CL3_ExtendedMessageFormat_Format(const char *pattern, const char *const *values, size_t count);

// Java: org.apache.commons.lang3.StringUtils join/split equivalents used by FML
char *LIBMATTI_CL3_StringUtils_Join(const char *const *parts, size_t count, char separator); // caller frees

#endif //MATTICRAFT_COMMONS_LANG3_TEXT_H
