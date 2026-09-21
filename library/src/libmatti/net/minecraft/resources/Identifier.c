// Port of net.minecraft.resources.Identifier.

#include "libmatti/net/minecraft/resources/Identifier.h"

#include "libmatti/net/minecraft/IdentifierException.h"
#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private Identifier(String namespace, String path) - the port copies
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_New(const char *namespace_, const char *path)
{
    LIBMATTI_MC_Identifier *identifier = calloc(1, sizeof(LIBMATTI_MC_Identifier));
    identifier->namespace = strdup(namespace_);
    identifier->path = strdup(path);
    return identifier;
}

void LIBMATTI_MC_Identifier_Free(LIBMATTI_MC_Identifier *identifier)
{
    if (identifier == NULL) return;
    free(identifier->namespace);
    free(identifier->path);
    free(identifier);
}

// Java: private static String assertValidNamespace(String namespace, String path)
static const char *assert_valid_namespace(const char *namespace_, const char *path)
{
    if (!LIBMATTI_MC_Identifier_IsValidNamespace(namespace_))
    {
        char message[512];
        snprintf(message, sizeof(message),
                 "Non [a-z0-9_.-] character in namespace of location: %s:%s", namespace_, path);
        LIBMATTI_JL_Throwable *exception = LIBMATTI_MC_IdentifierException_New(message);
        LIBMATTI_JL_Throwable_PrintStackTrace(exception);
        LIBMATTI_JL_Throwable_Free(exception);
        exit(1);
    }
    return namespace_;
}

// Java: private static String assertValidPath(String namespace, String path)
static const char *assert_valid_path(const char *namespace_, const char *path)
{
    if (!LIBMATTI_MC_Identifier_IsValidPath(path))
    {
        char message[512];
        snprintf(message, sizeof(message),
                 "Non [a-z0-9/._-] character in path of location: %s:%s", namespace_, path);
        LIBMATTI_JL_Throwable *exception = LIBMATTI_MC_IdentifierException_New(message);
        LIBMATTI_JL_Throwable_PrintStackTrace(exception);
        LIBMATTI_JL_Throwable_Free(exception);
        exit(1);
    }
    return path;
}

// Java: private static Identifier createUntrusted(String namespace, String path)
static LIBMATTI_MC_Identifier *create_untrusted(const char *namespace_, const char *path)
{
    return LIBMATTI_MC_Identifier_New(assert_valid_namespace(namespace_, path),
                                      assert_valid_path(namespace_, path));
}

// Java: public static Identifier fromNamespaceAndPath(String namespace, String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_FromNamespaceAndPath(const char *namespace_, const char *path)
{
    return create_untrusted(namespace_, path);
}

// Java: public static Identifier withDefaultNamespace(String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithDefaultNamespace(const char *path)
{
    assert_valid_path(LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE, path);
    return LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE, path);
}

// Java: public static Identifier bySeparator(String location, char separator)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_BySeparator(const char *location, char separator)
{
    const char *i = strchr(location, separator);
    if (i != NULL)
    {
        const char *s = i + 1;
        if (i != location)
        {
            size_t namespaceLength = (size_t) (i - location);
            char *namespace_ = malloc(namespaceLength + 1);
            memcpy(namespace_, location, namespaceLength);
            namespace_[namespaceLength] = '\0';
            LIBMATTI_MC_Identifier *identifier = create_untrusted(namespace_, s);
            free(namespace_);
            return identifier;
        }
        return LIBMATTI_MC_Identifier_WithDefaultNamespace(s);
    }
    return LIBMATTI_MC_Identifier_WithDefaultNamespace(location);
}

// Java: public static Identifier parse(String location)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_Parse(const char *location)
{
    return LIBMATTI_MC_Identifier_BySeparator(location, ':');
}

// Java: public static @Nullable Identifier tryBySeparator(String location, char separator)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryBySeparator(const char *location, char separator)
{
    const char *i = strchr(location, separator);
    if (i != NULL)
    {
        const char *s = i + 1;
        if (!LIBMATTI_MC_Identifier_IsValidPath(s))
            return NULL;
        if (i != location)
        {
            size_t namespaceLength = (size_t) (i - location);
            char *namespace_ = malloc(namespaceLength + 1);
            memcpy(namespace_, location, namespaceLength);
            namespace_[namespaceLength] = '\0';
            LIBMATTI_MC_Identifier *identifier =
                LIBMATTI_MC_Identifier_IsValidNamespace(namespace_)
                    ? LIBMATTI_MC_Identifier_New(namespace_, s) : NULL;
            free(namespace_);
            return identifier;
        }
        return LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE, s);
    }
    return LIBMATTI_MC_Identifier_IsValidPath(location)
               ? LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE, location) : NULL;
}

// Java: public static @Nullable Identifier tryParse(String location)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryParse(const char *location)
{
    return LIBMATTI_MC_Identifier_TryBySeparator(location, ':');
}

// Java: public static @Nullable Identifier tryBuild(String namespace, String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryBuild(const char *namespace_, const char *path)
{
    return LIBMATTI_MC_Identifier_IsValidNamespace(namespace_) && LIBMATTI_MC_Identifier_IsValidPath(path)
               ? LIBMATTI_MC_Identifier_New(namespace_, path) : NULL;
}

// Java: public String getPath()
const char *LIBMATTI_MC_Identifier_GetPath(const LIBMATTI_MC_Identifier *identifier)
{
    return identifier->path;
}

// Java: public String getNamespace()
const char *LIBMATTI_MC_Identifier_GetNamespace(const LIBMATTI_MC_Identifier *identifier)
{
    return identifier->namespace;
}

// Java: public Identifier withPath(String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithPath(const LIBMATTI_MC_Identifier *identifier, const char *path)
{
    assert_valid_path(identifier->namespace, path);
    return LIBMATTI_MC_Identifier_New(identifier->namespace, path);
}

// Java: public Identifier withPrefix(String prefix) - withPath(prefix + path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithPrefix(const LIBMATTI_MC_Identifier *identifier, const char *prefix)
{
    char *combined = malloc(strlen(prefix) + strlen(identifier->path) + 1);
    strcpy(combined, prefix);
    strcat(combined, identifier->path);
    LIBMATTI_MC_Identifier *result = LIBMATTI_MC_Identifier_WithPath(identifier, combined);
    free(combined);
    return result;
}

// Java: public Identifier withSuffix(String suffix) - withPath(path + suffix)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithSuffix(const LIBMATTI_MC_Identifier *identifier, const char *suffix)
{
    char *combined = malloc(strlen(identifier->path) + strlen(suffix) + 1);
    strcpy(combined, identifier->path);
    strcat(combined, suffix);
    LIBMATTI_MC_Identifier *result = LIBMATTI_MC_Identifier_WithPath(identifier, combined);
    free(combined);
    return result;
}

// Java: public String toString()
char *LIBMATTI_MC_Identifier_ToString(const LIBMATTI_MC_Identifier *identifier)
{
    size_t size = strlen(identifier->namespace) + 1 + strlen(identifier->path) + 1;
    char *string = malloc(size);
    snprintf(string, size, "%s:%s", identifier->namespace, identifier->path);
    return string;
}

// Java: public boolean equals(Object)
int LIBMATTI_MC_Identifier_Equals(const LIBMATTI_MC_Identifier *a, const LIBMATTI_MC_Identifier *b)
{
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    return strcmp(a->namespace, b->namespace) == 0 && strcmp(a->path, b->path) == 0;
}

// Java: public int hashCode() - String.hashCode is h*31+c over UTF-16 units
int LIBMATTI_MC_Identifier_HashCode(const LIBMATTI_MC_Identifier *identifier)
{
    int namespaceHash = 0;
    for (const unsigned char *p = (const unsigned char *) identifier->namespace; *p != '\0'; p++)
        namespaceHash = namespaceHash * 31 + *p;
    int pathHash = 0;
    for (const unsigned char *p = (const unsigned char *) identifier->path; *p != '\0'; p++)
        pathHash = pathHash * 31 + *p;
    return 31 * namespaceHash + pathHash;
}

// Java: public int compareTo(Identifier) - String.compareTo is the UTF-16 difference
int LIBMATTI_MC_Identifier_CompareTo(const LIBMATTI_MC_Identifier *a, const LIBMATTI_MC_Identifier *b)
{
    int i = strcmp(a->path, b->path);
    if (i == 0)
        i = strcmp(a->namespace, b->namespace);
    return i;
}

// Java: public String toDebugFileName()
char *LIBMATTI_MC_Identifier_ToDebugFileName(const LIBMATTI_MC_Identifier *identifier)
{
    char *string = LIBMATTI_MC_Identifier_ToString(identifier);
    for (char *p = string; *p != '\0'; p++)
        if (*p == '/' || *p == ':')
            *p = '_';
    return string;
}

// Java: public String toLanguageKey() - namespace + "." + path
char *LIBMATTI_MC_Identifier_ToLanguageKey(const LIBMATTI_MC_Identifier *identifier)
{
    size_t size = strlen(identifier->namespace) + 1 + strlen(identifier->path) + 1;
    char *key = malloc(size);
    snprintf(key, size, "%s.%s", identifier->namespace, identifier->path);
    return key;
}

// Java: public String toShortLanguageKey() - path when the namespace is minecraft
char *LIBMATTI_MC_Identifier_ToShortLanguageKey(const LIBMATTI_MC_Identifier *identifier)
{
    if (strcmp(identifier->namespace, LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE) == 0)
        return strdup(identifier->path);
    return LIBMATTI_MC_Identifier_ToLanguageKey(identifier);
}

// Java: public String toShortString()
char *LIBMATTI_MC_Identifier_ToShortString(const LIBMATTI_MC_Identifier *identifier)
{
    if (strcmp(identifier->namespace, LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE) == 0)
        return strdup(identifier->path);
    return LIBMATTI_MC_Identifier_ToString(identifier);
}

// Java: public String toLanguageKey(String prefix) - prefix + "." + toLanguageKey()
char *LIBMATTI_MC_Identifier_ToLanguageKeyWithPrefix(const LIBMATTI_MC_Identifier *identifier, const char *prefix)
{
    char *key = LIBMATTI_MC_Identifier_ToLanguageKey(identifier);
    size_t size = strlen(prefix) + 1 + strlen(key) + 1;
    char *result = malloc(size);
    snprintf(result, size, "%s.%s", prefix, key);
    free(key);
    return result;
}

// Java: public String toLanguageKey(String prefix, String suffix)
char *LIBMATTI_MC_Identifier_ToLanguageKeyWithPrefixAndSuffix(const LIBMATTI_MC_Identifier *identifier,
                                                              const char *prefix, const char *suffix)
{
    char *key = LIBMATTI_MC_Identifier_ToLanguageKey(identifier);
    size_t size = strlen(prefix) + 1 + strlen(key) + 1 + strlen(suffix) + 1;
    char *result = malloc(size);
    snprintf(result, size, "%s.%s.%s", prefix, key, suffix);
    free(key);
    return result;
}

// Java: public static boolean isAllowedInIdentifier(char)
int LIBMATTI_MC_Identifier_IsAllowedInIdentifier(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '_' || c == ':' || c == '/' || c == '.' || c == '-';
}

// Java: public static boolean validPathChar(char)
int LIBMATTI_MC_Identifier_ValidPathChar(char c)
{
    return c == '_' || c == '-' || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '/' || c == '.';
}

// Java: private static boolean validNamespaceChar(char)
int LIBMATTI_MC_Identifier_ValidNamespaceChar(char c)
{
    return c == '_' || c == '-' || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.';
}

// Java: public static boolean isValidPath(String)
int LIBMATTI_MC_Identifier_IsValidPath(const char *path)
{
    for (const char *p = path; *p != '\0'; p++)
        if (!LIBMATTI_MC_Identifier_ValidPathChar(*p))
            return 0;
    return 1;
}

// Java: public static boolean isValidNamespace(String)
int LIBMATTI_MC_Identifier_IsValidNamespace(const char *namespace_)
{
    for (const char *p = namespace_; *p != '\0'; p++)
        if (!LIBMATTI_MC_Identifier_ValidNamespaceChar(*p))
            return 0;
    return 1;
}
