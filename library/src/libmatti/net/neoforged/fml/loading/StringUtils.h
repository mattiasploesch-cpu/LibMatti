// Port of net.neoforged.fml.loading.StringUtils.
// parseStringFormat resolves through the ported org.apache.commons.lang3.text.StrSubstitutor.

#ifndef MATTICRAFT_FML_LOADING_STRINGUTILS_H
#define MATTICRAFT_FML_LOADING_STRINGUTILS_H

#include <stddef.h>

// Java: public static String toLowerCase(String str) - returns a new string
char *LIBMATTI_FML_StringUtils_ToLowerCase(const char *str);
// Java: public static String toUpperCase(String str) - returns a new string
char *LIBMATTI_FML_StringUtils_ToUpperCase(const char *str);

// Java: public static boolean endsWith(String search, String... endings)
int LIBMATTI_FML_StringUtils_EndsWith(const char *search, const char **endings, size_t endingCount);

// Java: public static URL toURL(String string) - a new URL handle, or NULL
void *LIBMATTI_FML_StringUtils_ToURL(const char *string);

// Java: public static String parseStringFormat(String input, Map<String, String> properties)
// The port's properties map is LIBMATTI_JU_HashMap (char* -> char*); caller frees.
char *LIBMATTI_FML_StringUtils_ParseStringFormat(const char *input, void *properties);

// Java: public static String binToHex(byte[] bytes) - returns a new string
char *LIBMATTI_FML_StringUtils_BinToHex(const unsigned char *bytes, size_t length);

#endif //MATTICRAFT_FML_LOADING_STRINGUTILS_H
