// Port of net.neoforged.fml.jarmoduleinfo.JlsConstants.
// Java uses Character.isJavaIdentifierStart/Part, which is defined over the full
// Unicode tables. The port implements the ASCII rules.

#ifndef MATTICRAFT_FML_JARMODULEINFO_JLSCONSTANTS_H
#define MATTICRAFT_FML_JARMODULEINFO_JLSCONSTANTS_H

#include <stddef.h>

// Java: static final Set<String> RESERVED_KEYWORDS
extern const char *const LIBMATTI_FML_JlsConstants_RESERVED_KEYWORDS[];
extern const size_t LIBMATTI_FML_JlsConstants_RESERVED_KEYWORD_COUNT;

// Java: public static boolean isJavaIdentifier(String str)
int LIBMATTI_FML_JlsConstants_IsJavaIdentifier(const char *str);
// Java: public static String getPackageName(String typeName) - returns a new string
char *LIBMATTI_FML_JlsConstants_GetPackageName(const char *typeName);
// Java: public static boolean isTypeName(String name)
int LIBMATTI_FML_JlsConstants_IsTypeName(const char *name);

#endif //MATTICRAFT_FML_JARMODULEINFO_JLSCONSTANTS_H
