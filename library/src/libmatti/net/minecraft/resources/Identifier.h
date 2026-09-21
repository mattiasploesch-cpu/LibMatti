// Port of net.minecraft.resources.Identifier.
// Java's String fields become char* copies owned by the identifier; the Codec/StreamCodec/
// Brigadier parts are out of scope (their owners are not ported yet) - the parse/validate
// core is complete 1:1.

#ifndef MATTICRAFT_NET_MINECRAFT_RESOURCES_IDENTIFIER_H
#define MATTICRAFT_NET_MINECRAFT_RESOURCES_IDENTIFIER_H

#include <stddef.h>

// Java: public final class Identifier implements Comparable<Identifier>
typedef struct LIBMATTI_MC_Identifier
{
    // Java: private final String namespace;
    char *namespace;
    // Java: private final String path;
    char *path;
} LIBMATTI_MC_Identifier;

// Java: public static final char NAMESPACE_SEPARATOR = ':'
#define LIBMATTI_MC_Identifier_NAMESPACE_SEPARATOR ':'
// Java: public static final String DEFAULT_NAMESPACE = "minecraft"
#define LIBMATTI_MC_Identifier_DEFAULT_NAMESPACE "minecraft"
// Java: public static final String REALMS_NAMESPACE = "realms"
#define LIBMATTI_MC_Identifier_REALMS_NAMESPACE "realms"

// Java: public static Identifier fromNamespaceAndPath(String namespace, String path) -
// validates both like createUntrusted and aborts the process on an invalid identifier
// (Java throws IdentifierException; the port reports and exits)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_FromNamespaceAndPath(const char *namespace_, const char *path);
// Java: public static Identifier parse(String location) - bySeparator(location, ':')
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_Parse(const char *location);
// Java: public static Identifier withDefaultNamespace(String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithDefaultNamespace(const char *path);
// Java: public static @Nullable Identifier tryParse(String location) - NULL when invalid
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryParse(const char *location);
// Java: public static @Nullable Identifier tryBuild(String namespace, String path)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryBuild(const char *namespace_, const char *path);
// Java: public static Identifier bySeparator(String location, char separator)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_BySeparator(const char *location, char separator);
// Java: public static @Nullable Identifier tryBySeparator(String location, char separator)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_TryBySeparator(const char *location, char separator);

// Java: public String getPath() / getNamespace()
const char *LIBMATTI_MC_Identifier_GetPath(const LIBMATTI_MC_Identifier *identifier);
const char *LIBMATTI_MC_Identifier_GetNamespace(const LIBMATTI_MC_Identifier *identifier);

// Java: public Identifier withPath(String path) / withPrefix(String) / withSuffix(String)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithPath(const LIBMATTI_MC_Identifier *identifier, const char *path);
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithPrefix(const LIBMATTI_MC_Identifier *identifier, const char *prefix);
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_WithSuffix(const LIBMATTI_MC_Identifier *identifier, const char *suffix);

// Java: public String toString() - "namespace:path"; the caller frees the string
char *LIBMATTI_MC_Identifier_ToString(const LIBMATTI_MC_Identifier *identifier);
// Java: public boolean equals(Object) - namespace and path equality (NULL is never equal)
int LIBMATTI_MC_Identifier_Equals(const LIBMATTI_MC_Identifier *a, const LIBMATTI_MC_Identifier *b);
// Java: public int hashCode() - 31 * namespace.hashCode() + path.hashCode()
int LIBMATTI_MC_Identifier_HashCode(const LIBMATTI_MC_Identifier *identifier);
// Java: public int compareTo(Identifier) - by path first, then namespace
int LIBMATTI_MC_Identifier_CompareTo(const LIBMATTI_MC_Identifier *a, const LIBMATTI_MC_Identifier *b);

// Java: public String toDebugFileName() - '/' and ':' become '_'; caller frees
char *LIBMATTI_MC_Identifier_ToDebugFileName(const LIBMATTI_MC_Identifier *identifier);
// Java: public String toLanguageKey() - "namespace.path"; caller frees
char *LIBMATTI_MC_Identifier_ToLanguageKey(const LIBMATTI_MC_Identifier *identifier);
// Java: public String toShortLanguageKey() - path only for the minecraft namespace
char *LIBMATTI_MC_Identifier_ToShortLanguageKey(const LIBMATTI_MC_Identifier *identifier);
// Java: public String toShortString() - path only for the minecraft namespace
char *LIBMATTI_MC_Identifier_ToShortString(const LIBMATTI_MC_Identifier *identifier);
// Java: public String toLanguageKey(String prefix) - "prefix.namespace.path"
char *LIBMATTI_MC_Identifier_ToLanguageKeyWithPrefix(const LIBMATTI_MC_Identifier *identifier, const char *prefix);
// Java: public String toLanguageKey(String prefix, String suffix) - "prefix.namespace.path.suffix"
char *LIBMATTI_MC_Identifier_ToLanguageKeyWithPrefixAndSuffix(const LIBMATTI_MC_Identifier *identifier,
                                                              const char *prefix, const char *suffix);

// Java: public static boolean isAllowedInIdentifier(char)
int LIBMATTI_MC_Identifier_IsAllowedInIdentifier(char c);
// Java: public static boolean isValidPath(String)
int LIBMATTI_MC_Identifier_IsValidPath(const char *path);
// Java: public static boolean isValidNamespace(String)
int LIBMATTI_MC_Identifier_IsValidNamespace(const char *namespace_);
// Java: public static boolean validPathChar(char)
int LIBMATTI_MC_Identifier_ValidPathChar(char c);
// Java: private static boolean validNamespaceChar(char)
int LIBMATTI_MC_Identifier_ValidNamespaceChar(char c);

// Java: the private constructor copies the strings (asserts in Java, the callers validate)
LIBMATTI_MC_Identifier *LIBMATTI_MC_Identifier_New(const char *namespace_, const char *path);
void LIBMATTI_MC_Identifier_Free(LIBMATTI_MC_Identifier *identifier);

#endif //MATTICRAFT_NET_MINECRAFT_RESOURCES_IDENTIFIER_H
