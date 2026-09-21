#include "libmatti/net/neoforged/fml/loading/StringUtils.h"

#include "libmatti/java/net/URL.h"
#include "libmatti/java/util/HashMap.h"
#include "libmatti/org/apache/commons/lang3/text/StrSubstitutor.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Java: public static String toLowerCase(String str)
char *LIBMATTI_FML_StringUtils_ToLowerCase(const char *str)
{
    // Java: str.toLowerCase(Locale.ROOT)
    char *result = strdup(str);
    for (char *p = result; *p != '\0'; p++) *p = (char) tolower((unsigned char) *p);
    return result;
}

// Java: public static String toUpperCase(String str)
char *LIBMATTI_FML_StringUtils_ToUpperCase(const char *str)
{
    // Java: str.toUpperCase(Locale.ROOT)
    char *result = strdup(str);
    for (char *p = result; *p != '\0'; p++) *p = (char) toupper((unsigned char) *p);
    return result;
}

// Java: public static boolean endsWith(String search, String... endings)
int LIBMATTI_FML_StringUtils_EndsWith(const char *search, const char **endings, size_t endingCount)
{
    char *lowerSearch = LIBMATTI_FML_StringUtils_ToLowerCase(search);
    int result = 0;
    for (size_t i = 0; i < endingCount && !result; i++)
    {
        size_t searchLength = strlen(lowerSearch);
        size_t endingLength = strlen(endings[i]);
        result = endingLength <= searchLength && strcmp(lowerSearch + searchLength - endingLength, endings[i]) == 0;
    }
    free(lowerSearch);
    return result;
}

// Java: public static URL toURL(String string)
void *LIBMATTI_FML_StringUtils_ToURL(const char *string)
{
    // Java: if (string == null || string.trim().isEmpty() || string.contains("myurl.me") || string.contains("example.invalid")) return null;
    if (string == NULL)
        return NULL;

    size_t length = strlen(string);
    size_t start = 0;
    while (start < length && isspace((unsigned char) string[start])) start++;
    if (start == length)
        return NULL;

    if (strstr(string, "myurl.me") != NULL || strstr(string, "example.invalid") != NULL)
        return NULL;

    LIBMATTI_JN_URL *url = LIBMATTI_JN_URL_New(string);
    // Java: catch (MalformedURLException e) { throw new RuntimeException(e); }
    return url;
}

// Java: public static String parseStringFormat(String input, Map<String, String> properties)
// The C port's property map is the libmatti HashMap<char*, char*> (keys and values
// are the strings); the lookup resolves through it.
static char *properties_lookup(const char *variable, void *self)
{
    LIBMATTI_JU_HashMap *properties = self;
    return (char *) LIBMATTI_JU_HashMap_Get(properties, variable);
}

char *LIBMATTI_FML_StringUtils_ParseStringFormat(const char *input, void *properties)
{
    // Java: return StrSubstitutor.replace(input, properties);
    return LIBMATTI_CL3_StrSubstitutor_Replace(input, properties_lookup, properties);
}

// Java: public static String binToHex(byte[] bytes)
char *LIBMATTI_FML_StringUtils_BinToHex(const unsigned char *bytes, size_t length)
{
    static const char *hex = "0123456789abcdef";
    char *result = malloc(length * 2 + 1);
    for (size_t i = 0; i < length; i++)
    {
        result[i * 2] = hex[(bytes[i] & 0xf0) >> 4];
        result[i * 2 + 1] = hex[bytes[i] & 0x0f];
    }
    result[length * 2] = '\0';
    return result;
}
